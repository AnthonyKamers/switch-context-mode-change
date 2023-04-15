#include "config.h"
#include "print.h"
#include "utils.h"
#include "../mmu/mmu.h"
#include "../mmu/mem.h"
#include "../mmu/kmem.h"
#include "../mmu/cache.h"

#define MAX_PROCESSES 2
#define MAX_STACK 1024
#define NEXT_PROCESS(i) (i + 1 % MAX_PROCESSES)

extern "C" void halt();
extern char* KERNEL_TABLE;
extern "C" void after_context_switch(volatile uint64_t **from_sp, volatile uint64_t **to_sp);

// PCB
typedef struct stack {
    volatile uint64_t * stack;
    volatile uint64_t * satp;
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

extern "C" void init_kernel_stack(volatile uint64_t * satp) {
    kernel_stack.satp = satp;
    kernel_stack.stack = kernel_stack.stack_base + (MAX_STACK - 1);
    asm("mv sp, %0" : : "r"(kernel_stack.stack):);
}

extern "C" void init_process() {
    const int main_id = 0;
    scheduler.length = 1;
    scheduler.current_id = main_id;
    scheduler.process[main_id].stack = scheduler.process[main_id].stack_base + (MAX_STACK - 1);
    asm("mv sp, %0" : : "r"(scheduler.process[main_id].stack):);
}

extern "C" void schedule() {
    int current_id = scheduler.current_id;
    int next_id = NEXT_PROCESS(current_id);

    // current sp (old process)
    uint64_t* sp;
    get_sp(sp);
    scheduler.process[next_id].stack = sp;

    // print SATP of next process
    int satp = *(scheduler.process[next_id].stack + 16);
    print("SATP: ", satp);

    // do context switch
    scheduler.current_id = next_id;

//    asm("mv a2, ra");
    after_context_switch(
            &scheduler.process[current_id].stack,
            &scheduler.process[next_id].stack);
}

void create_process(void (*process_entry)(void), uint64_t satp) {
    unsigned int id = scheduler.length;
    scheduler.process[id].stack = scheduler.process[id].stack_base + (MAX_STACK - 1);

    // push arguments to the stack
    *scheduler.process[id].stack-- = (uint64_t) process_entry;                  // PC
    *scheduler.process[id].stack-- = satp;                                      // SATP
    *scheduler.process[id].stack-- = 0;                                        // t6
    *scheduler.process[id].stack-- = 0;                                        // t5
    *scheduler.process[id].stack-- = 0;                                        // t4
    *scheduler.process[id].stack-- = 0;                                        // t3
    *scheduler.process[id].stack-- = 0;                                        // t2
    *scheduler.process[id].stack-- = 0;                                        // t1
    *scheduler.process[id].stack-- = 0;                                        // t0
    *scheduler.process[id].stack-- = 0;                                        // a7
    *scheduler.process[id].stack-- = 0;                                        // a6
    *scheduler.process[id].stack-- = 0;                                        // a5
    *scheduler.process[id].stack-- = 0;                                        // a4
    *scheduler.process[id].stack-- = 0;                                        // a3
    *scheduler.process[id].stack-- = 0;                                        // a2
    *scheduler.process[id].stack-- = 0;                                        // a1
    *scheduler.process[id].stack-- = 0;                                        // a0
    *scheduler.process[id].stack = 0;                                          // ra

    // increase scheduler process length
    scheduler.length++;
}

void process1_entry(void) {
    const char * message = "process1";
    while (TRUE)
        print(message);
}

void process2_entry(void) {
    const char * message = "process2";
    while (TRUE)
        print(message);
}

//void make_process() {
//    char * page_allocated = zalloc(64);
//    uint64_t satp_transfer = char_to_satp(page_allocated);
//    set_satp(satp_transfer);
//}

extern "C" int kinit() {
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

    // make structure data to keep kernel data;
//    kernel_stack.stack = get_sp();
//    kernel_stack.satp = (volatile uint64_t *) satp_kernel;

    // return the address of kernel table page (to satp)
    return satp_kernel;
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