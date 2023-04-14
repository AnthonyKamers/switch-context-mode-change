#ifndef MEM
#define MEM
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../c/utils.h"
#define PAGE_SIZE 4096

    extern uint64_t HEAP_START;
    extern uint64_t HEAP_SIZE;
    extern uint64_t TEXT_START;
    extern uint64_t TEXT_END;
    extern uint64_t DATA_START;
    extern uint64_t DATA_END;
    extern uint64_t RODATA_START;
    extern uint64_t RODATA_END;
    extern uint64_t BSS_START;
    extern uint64_t BSS_END;
    extern uint64_t KERNEL_STACK_START;
    extern uint64_t KERNEL_STACK_END;

    extern uint64_t ALLOC_START;


uint64_t align_val(uint64_t val, uint64_t order){
	uint64_t o = (1 << order) - 1;
	return(val + o) & ~o;
}

// We will use ALLOC_START to mark the start of the actual
// memory we can dish out.

const uint64_t PAGE_ORDER = 12;

/// Align (set to a multiple of some power of two)
/// This takes an order which is the exponent to 2^order
/// Therefore, all alignments must be made as a power of two.
/// This function always rounds up.
enum page_bits {
	empty = 0,
	taken = 1 << 0,
	last = 1 << 1,
} page_bits;


// Each page is described by the Page structure. Linux does this
// as well, where each 4096-byte chunk of memory has a structure
// associated with it. However, there structure is much larger.
typedef struct {
	char flags;
}page;

bool is_last(page p){
    if (p.flags & ((char) last) != 0) {
        return true;
    }
    else {
        return false;
    }
}

// If the page is marked as being taken (allocated), then
// this function returns true. Otherwise, it returns false.
bool is_taken(page p){
    if (p.flags & ((char) taken) != 0) {
        return true;
    }
    else {
        return false;
    }
}

// This is the opposite of is_taken().
bool is_free(page p){
    return !is_taken(p);
}

// Clear the Page structure and all associated allocations.
void clear(page* p){
    p->flags = (char)empty;
}

// Set a certain flag. We ran into trouble here since PageBits
// is an enumeration and we haven't implemented the BitOr Trait
// on it.
void set_flag(page* p, char flag) {
    p->flags |= flag;
}

void clear_flag(page* p, char flag) {
    p->flags &= !(flag);
}


/// Initialize the allocation system. There are several ways that we can
/// implement the page allocator:
/// 1. Free list (singly linked list where it starts at the first free
/// allocation)
/// 2. Bookkeeping list (structure contains a taken and length)
/// 3. Allocate one Page structure per 4096 bytes (this is what I chose)
/// 4. Others
void page_init() {
		uint64_t num_pages = HEAP_SIZE / PAGE_SIZE;
		page* ptr = (page*) HEAP_START;
		// Clear all pages to make sure that they aren't accidentally
		// taken
		for (int i = 0; i<num_pages; i++){
			clear(ptr+i);
		}
		// // Determine where the actual useful memory starts. This will be
		// // after all Page structures. We also must align the ALLOC_START
		// // to a page-boundary (PAGE_SIZE = 4096). ALLOC_START =
		// // (HEAP_START + num_pages * size_of::<Page>() + PAGE_SIZE - 1)
		// // & !(PAGE_SIZE - 1);
		ALLOC_START = align_val(HEAP_START + num_pages * sizeof(page), PAGE_ORDER);
}

/// Allocate a page or multiple pages
/// pages: the number of PAGE_SIZE pages to allocate
char* alloc(uint64_t pages){
	// We have to find a contiguous allocation of pages
    // We create a Page structure for each page on the heap. We
    // actually might have more since HEAP_SIZE moves and so does
    // the size of our structure, but we'll only waste a few bytes.
    uint64_t num_pages = HEAP_SIZE / PAGE_SIZE;
    page* ptr = (page*)HEAP_START;
    for (int i=0; i<num_pages-pages; i++){
        bool found = false;
        // Check to see if this Page is free. If so, we have our
        // first candidate memory address.
        if (is_free(*(ptr+i))) {
            // It was FREE! Yay!
            found = true;
            for (int j=i; j < (i+pages); j++) {
                // Now check to see if we have a
                // contiguous allocation for all of the
                // request pages. If not, we should
                // check somewhere else.
                if (is_taken(*(ptr + j))) {
                    found = false;
                    break;
                }
            }
        }
        // We've checked to see if there are enough contiguous
        // pages to form what we need. If we couldn't, found
        // will be false, otherwise it will be true, which means
        // we've found valid memory we can allocate.
        if (found) {
            for (int k = i; k<(i + pages - 1); k++) {
                set_flag(ptr + k, taken);
            }
            // The marker for the last page is
            // PageBits::Last This lets us know when we've
            // hit the end of this particular allocation.

            set_flag(ptr + i+pages-1, taken);
            set_flag(ptr + i+pages-1, last);
            // The Page structures themselves aren't the
            // useful memory. Instead, there is 1 Page
            // structure per 4096 bytes starting at
            // ALLOC_START.
            return (char*)(ALLOC_START + PAGE_SIZE * i);
        }
    }

	// If we get here, that means that no contiguous allocation was
	// found.
	return NULL;
}

/// Allocate and zero a page or multiple pages
/// pages: the number of pages to allocate
/// Each page is PAGE_SIZE which is calculated as 1 << PAGE_ORDER
/// On RISC-V, this typically will be 4,096 bytes.
char* zalloc(uint64_t pages) {
	// Allocate and zero a page.
	// First, let's get the allocation
	char* ret = alloc(pages);
	if (ret != NULL) {
		uint64_t size = (PAGE_SIZE * pages) / 8;
		uint64_t* big_ptr = (uint64_t*)ret;
		for (int i=0; i<size; i++){
			// We use big_ptr so that we can force an
			// sd (store doubleword) instruction rather than
			// the sb. This means 8x fewer stores than before.
			// Typically we have to be concerned about remaining
			// bytes, but fortunately 4096 % 8 = 0, so we
			// won't have any remaining bytes.
			*(big_ptr+i) = 0;
		}
	}
	return ret;
}

uint64_t make_process_pagetable() {
    char * page_allocated = zalloc(64);
    uint64_t satp_transfer = char_to_satp(page_allocated);
    return satp_transfer;
}

/// Deallocate a page by its pointer
/// The way we've structured this, it will automatically coalesce
/// contiguous pages.
void dealloc(char* ptr) {
	// Make sure we don't try to free a null pointer.
		uint64_t addr = HEAP_START + (uint64_t)(ptr - ALLOC_START) / PAGE_SIZE;
		// Make sure that the address makes sense. The address we
		// calculate here is the page structure, not the HEAP address!
		page* p = (page*)addr;
		// Keep clearing pages until we hit the last page.
		while (is_taken(*p) && !is_last(*p)) {
			clear(p);
			p = p+1;
		}
		// If the following assertion fails, it is most likely
		// caused by a double-free.
		// If we get here, we've taken care of all previous pages and
		// we are on the last page.
		clear(p);
}

/// Print all page allocations
/// This is mainly used for debugging.
// pub fn print_page_allocations() {
// 	unsafe {
// 		let num_pages = HEAP_SIZE / PAGE_SIZE;
// 		let mut beg = HEAP_START as *const Page;
// 		let end = beg.add(num_pages);
// 		let alloc_beg = ALLOC_START;
// 		let alloc_end = ALLOC_START + num_pages * PAGE_SIZE;
// 		println!();
// 		println!(
// 		         "PAGE ALLOCATION TABLE\nMETA: {:p} -> {:p}\nPHYS: \
// 		          0x{:x} -> 0x{:x}",
// 		         beg, end, alloc_beg, alloc_end
// 		);
// 		println!("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
// 		let mut num = 0;
// 		while beg < end {
// 			if (*beg).is_taken() {
// 				let start = beg as usize;
// 				let memaddr = ALLOC_START
// 				              + (start - HEAP_START)
// 				                * PAGE_SIZE;
// 				print!("0x{:x} => ", memaddr);
// 				loop {
// 					num += 1;
// 					if (*beg).is_last() {
// 						let end = beg as usize;
// 						let memaddr = ALLOC_START
// 						              + (end
// 						                 - HEAP_START)
// 						                * PAGE_SIZE
// 						              + PAGE_SIZE - 1;
// 						print!(
// 						       "0x{:x}: {:>3} page(s)",
// 						       memaddr,
// 						       (end - start + 1)
// 						);
// 						println!(".");
// 						break;
// 					}
// 					beg = beg.add(1);
// 				}
// 			}
// 			beg = beg.add(1);
// 		}
// 		println!("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
// 		println!(
// 		         "Allocated: {:>5} pages ({:>9} bytes).",
// 		         num,
// 		         num * PAGE_SIZE
// 		);
// 		println!(
// 		         "Free     : {:>5} pages ({:>9} bytes).",
// 		         num_pages - num,
// 		         (num_pages - num) * PAGE_SIZE
// 		);
// 		println!();
// 	}
// }
#endif
