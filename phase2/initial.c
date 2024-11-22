#ifndef INITIAL_C
#define INITIAL_C

#include "./headers/initial.h"

extern void test();
extern void uTLB_RefillHandler();
extern void GeneralExceptionHandler();
extern void SSIServer();

// variabili globali
int process_count;
int soft_blocked_count;
struct list_head ready_queue;
pcb_t* current_process;
pcb_t* ssi_pcb;
pcb_t* blocked_dev[SEMDEVLEN - 1];
struct list_head blocked_pseudo_clock;
struct list_head blocked_msg;
pcb_t* second; //spostato qui temporaneamente per motivi di debug


void main(){

    passupvector_t* puv = (passupvector_t*)PASSUPVECTOR;
    // Inizializzo il Passup Vector
    puv->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    puv->tlb_refill_stackPtr = KERNELSTACK;
    puv->exception_handler = (memaddr)GeneralExceptionHandler;
    puv->exception_stackPtr = KERNELSTACK;

    // Inizializzazione di PCB e messaggi
    initPcbs();
    initMsgs();

    // Inizializzazione variabili globali
    process_count = 0;
    soft_blocked_count = 0;
    mkEmptyProcQ(&ready_queue);
    current_process = NULL;
    for (int i = 0; i < SEMDEVLEN - 1; i++){
        blocked_dev[i] = NULL;
    }
    mkEmptyProcQ(&blocked_pseudo_clock);
    mkEmptyProcQ(&blocked_msg);

    // Carico l'interval timer
    LDIT(PSECOND);

    unsigned int ram_top;
    RAMTOP(ram_top);

    // Inizializzo l'SSI
    ssi_pcb = allocPcb();
    ssi_pcb->p_parent = NULL;
    INIT_LIST_HEAD(&ssi_pcb->p_sib);
    INIT_LIST_HEAD(&ssi_pcb->p_child);
    ssi_pcb->p_s.pc_epc=(memaddr)SSIServer;
    ssi_pcb->p_s.status = IEPON | IMON | TEBITON ;
    ssi_pcb->p_s.reg_sp = ram_top;
    ssi_pcb->p_time = 0;
    ssi_pcb->p_supportStruct = NULL;
    insertProcQ(&ready_queue, ssi_pcb);
    process_count++;

    // Inizializzo il processo di test
    //pcb_t* second = allocPcb();
    second = allocPcb();
    second->p_parent = NULL;
    INIT_LIST_HEAD(&second->p_sib);
    INIT_LIST_HEAD(&second->p_child);
    second->p_s.pc_epc=(memaddr)test;
    second->p_s.status = IEPON | IMON | TEBITON ;
    second->p_s.reg_sp = ram_top - (2 * PAGESIZE);
    second->p_time = 0;
    second->p_supportStruct = NULL;
    insertProcQ(&ready_queue, second);
    process_count++;

    // Passo il controllo allo scheduler
    scheduler();
}

#endif