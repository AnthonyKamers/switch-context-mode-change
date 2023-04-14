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

typedef struct stack {
    volatile uint64_t * stack;
    volatile uint64_t * satp;
    uint64_t stack_base[MAX_STACK];
} process_t;

typedef struct {
    unsigned int length;
    volatile unsigned int current_id;
    process_t process[MAX_PROCESSES];
} scheduler_t;

scheduler_t scheduler;
//extern "C" process_t kernel_stack;

uint64_t * _get_stack_pointer() {
    // load instructions (assembly) to take the actual sp
    return 0;
}

void after_context_switch(volatile unsigned int **old_sp, volatile unsigned int **to_sp) {
    // get registers from stack and clean it
}

void schedule() {
    int current_id = scheduler.current_id;
    int next_id = NEXT_PROCESS(current_id);

    // current sp
    scheduler.process[next_id].stack = _get_stack_pointer();

    // print SATP of next process
    int satp = *(scheduler.process[next_id].stack + 14);
    print(satp);

    // do context switch
    scheduler.current_id = next_id;
    // after_context_switch(); // fix and uncomment
}

void create_process(void *process_entry) {
    unsigned int id = scheduler.length;
    scheduler.process[id].stack = scheduler.process[id].stack_base + (MAX_STACK - 1);

    // push arguments to the stack
    *scheduler.process[id].stack-- = *((unsigned int*)(&process_entry));    // PC
    *scheduler.process[id].stack-- = get_mstatus();                         // mstatus
    *scheduler.process[id].stack-- = make_process_pagetable();              // SATP
    // *scheduler.task[id].stack-- = 0;                                        // ra
    *scheduler.process[id].stack-- = 0;                                        // s0
    *scheduler.process[id].stack-- = 0;                                        // s1
    *scheduler.process[id].stack-- = 0;                                        // a0
    *scheduler.process[id].stack-- = 0;                                        // a1
    *scheduler.process[id].stack-- = 0;                                        // a2
    *scheduler.process[id].stack-- = 0;                                        // a3
    *scheduler.process[id].stack-- = 0;                                        // a4

    // increase scheduler process length
    scheduler.length++;
}

int process1_entry() {
    const char * message = "process1";
    while (TRUE)
        print(message);
    return 0;
}

int process2_entry() {
    const char * message = "process2";
    while (TRUE)
        print(message);
    return 0;
}

void make_process() {
    char * page_allocated = zalloc(64);
    uint64_t satp_transfer = char_to_satp(page_allocated);
    set_satp(satp_transfer);
}

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
    const char * message = "main\n";
    print(message);

    // make_process();
//    create_process(process1_entry);
//    create_process(process2_entry);

    halt();
//    create_process(process1_entry);
//    create_process(process2_entry);
//    halt();
}