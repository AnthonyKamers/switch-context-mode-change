#include "config.h"
#include "print.h"
#include "utils.h"
#include "timer.h"
#include "../mmu/mmu.h"
#include "../mmu/mem.h"
#include "../mmu/kmem.h"
#include "../mmu/cache.h"

#define MAX_PROCESSES 3
#define MAX_STACK 1024

extern "C" void halt();
extern char* KERNEL_TABLE;
extern "C" void after_context_switch(volatile uint64_t *from_sp, volatile uint64_t *to_sp);
extern "C" void asm_create_process(volatile uint64_t * stack, uint64_t process_entry, uint64_t satp);

// PCB
typedef struct stack {
    volatile uint64_t * stack;
    uint64_t stack_base[MAX_STACK];
} process_t;

// Scheduler
typedef struct {
    unsigned int length;
    volatile unsigned int current_id;
    process_t process[MAX_PROCESSES];
} scheduler_t;

scheduler_t scheduler;
process_t kernel_stack;

extern "C" void switch_kernel_stack() {
    asm("mv sp, %0" : : "r"(kernel_stack.stack):);
}

extern "C" void switch_user_stack() {
    asm("mv sp, %0" : : "r"(scheduler.process[scheduler.current_id].stack):);
}

extern "C" void init_kernel_stack() {
    kernel_stack.stack = kernel_stack.stack_base + (MAX_STACK - 1);
    asm("mv sp, %0" : : "r"(kernel_stack.stack):);
}

extern "C" void init_process() {
    asm("la t0, 0x0");
    asm("mv sp, t0");

    const int main_id = 0;
    scheduler.length = 1;
    scheduler.current_id = main_id;
    scheduler.process[main_id].stack = scheduler.process[main_id].stack_base + (MAX_STACK - 1);
    asm("mv sp, %0" : : "r"(scheduler.process[main_id].stack):);
}

extern "C" void schedule() {
    int current_id = scheduler.current_id;
    int next_id = current_id + 1;

    // round-robin: circular queue
    if (next_id >= scheduler.length) next_id = 0;

    // current sp (old process)
    volatile uint64_t * sp = get_sp() + 4;
    scheduler.process[current_id].stack = sp;

    // do context switch
    scheduler.current_id = next_id;

    after_context_switch(
            scheduler.process[current_id].stack,
            scheduler.process[next_id].stack);
}

void create_process(void (*process_entry)(void), uint64_t satp) {
    unsigned int id = scheduler.length;
    scheduler.process[id].stack = scheduler.process[id].stack_base + (MAX_STACK - 1);

    // add first information into the stack
    asm_create_process(scheduler.process[id].stack, (uint64_t)process_entry, satp);

    // increase scheduler process length
    scheduler.length++;
}

void process1_entry(void) {
    const char * message = "process1\n";
    while (TRUE) {
        print(message);
        halt();
    }
}

void process2_entry(void) {
    const char * message = "process2\n";
    while (TRUE) {
        print(message);
        halt();
    }
}

extern "C" void kinit() {
    const char * message = "kinit\n";
    print(message);

    // page table initialization
    page_init();

    // kernel memory initilization
    kmem_init();
    table* root_ptr = get_page_table();
    u_int64_t root_size = sizeof(root_ptr);
    char* kheap_head = get_head();
    u_int64_t total_pages = get_num_allocations();
    id_map_range(root_ptr,(u_int64_t) kheap_head,(u_int64_t) kheap_head + total_pages * 4096, READ|WRITE);

    // set sizes
    u_int64_t num_pages = HEAP_SIZE / PAGE_SIZE;
    id_map_range(root_ptr,(u_int64_t) HEAP_START,(u_int64_t) HEAP_START + num_pages, READ|WRITE);
    id_map_range(root_ptr,(u_int64_t) TEXT_START,(u_int64_t) TEXT_END, READ|EXECUTE);
    id_map_range(root_ptr,(u_int64_t) RODATA_START,(u_int64_t) RODATA_END, READ|EXECUTE);
    id_map_range(root_ptr,(u_int64_t) DATA_START,(u_int64_t) DATA_END, READ|WRITE);
    id_map_range(root_ptr,(u_int64_t) BSS_START,(u_int64_t) BSS_END, READ|WRITE);
    id_map_range(root_ptr,(u_int64_t) KERNEL_STACK_START,(u_int64_t) KERNEL_STACK_END, READ|WRITE);

    KERNEL_TABLE = (char*)root_ptr;
    uint64_t satp_kernel = char_to_satp(KERNEL_TABLE);

    // Force the CPU to take our SATP register.
    // To be efficient, if the address space identifier (ASID) portion of SATP is already
    // in cache, it will just grab whatever's in cache. However, that means if we've updated
    // it in memory, it will be the old table. So, sfence.vma will ensure that the MMU always
    // grabs a fresh copy of the SATP register and associated tables.
    asm("sfence.vma");

    // set SATP
    set_satp(satp_kernel);
}

extern "C" int main() {
    uint64_t satp0 = make_process_pagetable();
    create_process(&process1_entry, satp0);

    uint64_t satp1 = make_process_pagetable();
    create_process(&process2_entry, satp1);

    while (TRUE) {
        print("0000000000\n");
        halt();
    }
    return 0;
}