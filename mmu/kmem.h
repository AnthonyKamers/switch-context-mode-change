#ifndef KMEM
#define KMEM
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mmu.h"

#define TAKEN 1ULL << 63


typedef struct {
	uint64_t flags_size;
}alloc_list;

bool is_taken(alloc_list alist){
    return alist.flags_size & TAKEN != 0;
}

bool is_free(alloc_list alist){
    return !is_taken(alist);
}

void set_taken(alloc_list* alist) {
    alist->flags_size |= TAKEN;
}

void set_free(alloc_list* alist) {
    alist->flags_size &= !TAKEN;
}

void set_size(alloc_list* alist, uint64_t sz) {
    bool k = is_taken(*alist);
    alist->flags_size = sz & ~TAKEN;
    if (k) {
        alist->flags_size |= TAKEN;
    }
}

uint64_t get_size(alloc_list alist){
    return alist.flags_size & !TAKEN;
}


// This is the head of the allocation. We start here when
// we search for a free memory location.
extern alloc_list*  KMEM_HEAD;
// In the future, we will have on-demand pages
// so, we need to keep track of our memory footprint to
// see if we actually need to allocate more.
extern uint64_t KMEM_ALLOC;
extern table* KMEM_PAGE_TABLE;


// These functions are safe helpers around an unsafe
// operation.
char* get_head(){
    return (char*) KMEM_HEAD;
}

table* get_page_table(){
	return (table*) KMEM_PAGE_TABLE;
}

uint64_t get_num_allocations(){
	return KMEM_ALLOC;
}

/// Initialize kernel's memory
/// This is not to be used to allocate memory
/// for user processes. If that's the case, use
/// alloc/dealloc from the page crate.
void kmem_init() {
		// Allocate 64 kernel pages (64 * 4096 = 262 KiB)
    char* k_alloc = zalloc(64);

    KMEM_ALLOC = 64;
    KMEM_HEAD = (alloc_list*)k_alloc;
    set_free(KMEM_HEAD);
    set_size(KMEM_HEAD, KMEM_ALLOC * PAGE_SIZE);
    KMEM_PAGE_TABLE = (table*) zalloc(1);

    asm volatile("addi t5, x0, 1");
}

char* kmalloc(uint64_t sz){
		uint64_t size = align_val(sz, 3) + sizeof(alloc_list);
		alloc_list* head = KMEM_HEAD;
		// .add() uses pointer arithmetic, so we type-cast into a u8
		// so that we multiply by an absolute size (KMEM_ALLOC *
		// PAGE_SIZE).
		alloc_list* tail = (alloc_list*)(KMEM_HEAD + (KMEM_ALLOC * PAGE_SIZE));

		while (head < tail) {
			if (is_free(*head) && size <= get_size(*head)) {
				uint64_t chunk_size = get_size(*head);
				uint64_t rem = chunk_size - size;
				set_taken(head);
				if (rem > sizeof(alloc_list)) {
					alloc_list* next = (alloc_list*)(head +size);
					set_free(next);
					set_size(next, rem);
					set_size(head,size);
				}
				else {
					// If we get here, take the entire chunk
					set_size(head,chunk_size);
				}
				return (char*) head+1;
			}
			else {
				// If we get here, what we saw wasn't a free
				// chunk, move on to the next.
				head = (alloc_list*) (head + (get_size(*head)));
			}
		}
	// If we get here, we didn't find any free chunks--i.e. there isn't
	// enough memory for this. TODO: Add on-demand page allocation.
	return NULL;
}

/// Allocate sub-page level allocation based on bytes and zero the memory
char* kzmalloc(uint64_t sz){
	uint64_t size = align_val(sz, 3);
	char* ret = kmalloc(size);

	if (ret!=NULL) {
		for (int i = 0; i< size; i++){
			(*(ret+i)) = 0;
		}
	}
	return ret;
}

/// Allocate sub-page level allocation based on bytes


void coalesce() {
		alloc_list* head = KMEM_HEAD;
		alloc_list* tail = (KMEM_HEAD+(KMEM_ALLOC * PAGE_SIZE));

		while (head < tail) {
			alloc_list* next = (head +(get_size(*head)));
			if (get_size(*head) == 0) {
				// If this happens, then we have a bad heap
				// (double free or something). However, that
				// will cause an infinite loop since the next
				// pointer will never move beyond the current
				// location.
				break;
			}
			else if (next >= tail) {
				// We calculated the next by using the size
				// given as get_size(), however this could push
				// us past the tail. In that case, the size is
				// wrong, hence we break and stop doing what we
				// need to do.
				break;
			}
			else if (is_free(*head) && is_free(*next)) {
				// This means we have adjacent blocks needing to
				// be freed. So, we combine them into one
				// allocation.
				set_size(head, get_size(*head) + get_size(*next));
			}
			// If we get here, we might've moved. Recalculate new
			// head.
			head = (head + (get_size(*head)));
		}
}

/// Free a sub-page level allocation
void kfree(char* ptr) {
    if (ptr != NULL) {
        alloc_list* p = (alloc_list*)(ptr-1);
        if (is_taken(*p)) {
            set_free(p);
        }
        // After we free, see if we can combine adjacent free
        // spots to see if we can reduce fragmentation.
        coalesce();
    }
}

/// Merge smaller chunks into a bigger chunk


/// For debugging purposes, print the kmem table
//pub fn print_table() {
// 	unsafe {
// 		let mut head = KMEM_HEAD;
// 		let tail = (KMEM_HEAD as *mut u8).add(KMEM_ALLOC * PAGE_SIZE)
// 		           as *mut AllocList;
// 		while head < tail {
// 			println!(
// 			         "{:p}: Length = {:<10} Taken = {}",
// 			         head,
// 			         (*head).get_size(),
// 			         (*head).is_taken()
// 			);
// 			head = (head as *mut u8).add((*head).get_size())
// 			       as *mut AllocList;
// 		}
// 	}
// }

// ///////////////////////////////////
// / GLOBAL ALLOCATOR
// ///////////////////////////////////

// The global allocator allows us to use the data structures
// in the core library, such as a linked list or B-tree.
// We want to use these sparingly since we have a coarse-grained
// allocator.
// use core::alloc::{GlobalAlloc, Layout};

// // The global allocator is a static constant to a global allocator
// // structure. We don't need any members because we're using this
// // structure just to implement alloc and dealloc.
// struct OsGlobalAlloc;

// unsafe impl GlobalAlloc for OsGlobalAlloc {
// 	unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
// 		// We align to the next page size so that when
// 		// we divide by PAGE_SIZE, we get exactly the number
// 		// of pages necessary.
// 		kzmalloc(layout.size())
// 	}

// 	unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
// 		// We ignore layout since our allocator uses ptr_start -> last
// 		// to determine the span of an allocation.
// 		kfree(ptr);
// 	}
// }

// #[global_allocator]
// /// Technically, we don't need the {} at the end, but it
// /// reveals that we're creating a new structure and not just
// /// copying a value.
// static GA: OsGlobalAlloc = OsGlobalAlloc {};

#endif
