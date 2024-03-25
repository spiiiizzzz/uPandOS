#include "/usr/include/umps3/umps/libumps.h"
#include "./phase1/headers/pcb.h"
#include "./phase1/headers/msg.h"
#include "./headers/types.h"
#include "const.h"

// global variables
int process_count;
int soft_blocked_count;
struct list_head ready_queue;
pcb_t* current_process;
pcb_t* ssi_process;
struct list_head blocked_dev[SEMDEVLEN - 1];
struct list_head blocked_pseudo_clock;
struct list_head blocked_msg;

extern void test();

int main(){
    // Initializing Passupvector of processor 0
    PASSUPVECTOR->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    PASSUPVECTOR->tlb_refill_stackPtr = KERNELSTACK;
    // PASSUPVECTOR->exception_handler = ...; TODO: add exception handler once it has been implemented
    PASSUPVECTOR->exception_stackPtr = KERNELSTACK;

    // Initializing pcbs and msgs
    initPcbs();
    initMsgs();

    // Initializing global variables
    process_count = 0;
    soft_blocked_count = 0;
    mkEmptyProcQ(&ready_queue);
    current_process = NULL;
    for (int i = 0; i < SEMDEVLEN - 1; i++){
        mkEmptyProcQ(&(blocked_dev[i]));
    }
    mkEmptyProcQ(&blocked_pseudo_clock);

    // Loading interval timer
    LDIT(PSECOND);

    ssi_process = allocPcb();
    // TODO: finish initialization
    ssi_process.p_parent = NULL;
    INIT_LIST_HEAD(ssi_process.p_sib);
    INIT_LIST_HEAD(ssi_process.p_child);
    // ssi_process.p_s.s_pc=&...;  TODO: complete once the ssi_handler is implemented
    ssi_process.p_s.s_status = IEPON | TEBITON ; // TODO: Complete
    ssi_process.p_time = 0;
    ssi_process.p_supportStruct = NULL;
    insertProcQ(ready_queue.p_list, ssi_process);

    pcb_t* second = allocPcb();
    // TODO: finish initialization
    second.p_parent = NULL;
    INIT_LIST_HEAD(second.p_sib);
    INIT_LIST_HEAD(second.p_child);
    second.p_s.s_pc=&test;
    second.p_s.s_status = IEPON | TEBITON ; // TODO: Complete
    second.p_time = 0;
    second.p_supportStruct = NULL;
    insertProcQ(ready_queue.p_list, second);


    // scheduler
    // TODO: complete once scheduler is implemented
}