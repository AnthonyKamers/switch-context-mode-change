#include "config.h"
#include "print.h"
#include "utils.h"
#include "timer.h"
#include "../mmu/mmu.h"
#include "../mmu/mem.h"
#include "../mmu/kmem.h"
#include "../mmu/cache.h"

#define MAX_PROCESSES 2
#define MAX_STACK 1024
#define NEXT_PROCESS(i) (i + 1 % MAX_PROCESSES)

extern "C" void halt();
extern char* KERNEL_TABLE;

//typedef struct stack {
//    volatile uint64_t * stack;
//    volatile uint64_t * satp;
//    uint64_t stack_base[MAX_STACK];
//} process_t;
//
//typedef struct {
//    unsigned int length;
//    volatile unsigned int current_id;
//    process_t process[MAX_PROCESSES];
//} scheduler_t;

uint8_t process_stack[MAX_PROCESSES][MAX_STACK];

typedef struct context {
    uint64_t ra;
    uint64_t sp;

    // callee-saved
    uint64_t s0;
    uint64_t s1;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
} context_t;

extern "C" void context_switch(context_t *context_old, context_t *context_new);

context_t context_os;
context_t context_process[MAX_PROCESSES];
context_t * context_now;
int task_top = 0;   // total number of processes

int create_process(void (*process)(void)) {
    int i = task_top++;
    context_process[i].ra = (uint64_t) process;
    context_process[i].sp = (uint64_t) &process_stack[i][MAX_STACK - 1];
    return i;
}

void process_go(int i) {
    context_now = &context_process[i];
    context_switch(&context_os, &context_process[i]);
}

void process_os() {
    context_t * context = context_now;
    context_now = &context_os;
    context_switch(context, &context_os);
}

void process1_entry(void) {
    const char * message = "process1";
    while (TRUE) {
        print(message);
        delay(0.5);
        process_os();
    }
}

void process2_entry(void) {
    const char * message = "process2";
    while (TRUE) {
        print(message);
        delay(0.5);
        process_os();
    }
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
    create_process(&process1_entry);
    create_process(&process2_entry);

    int current_process = 0;
    while (TRUE) {
        print("OS activate next process;\n");
        process_go(current_process);
        print("Back to OS\n");

        current_process = NEXT_PROCESS(current_process);
        print("\n");
    }

    halt();
    return 0;
}