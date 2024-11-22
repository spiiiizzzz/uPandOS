#ifndef INITPROC_C
#define INITPROC_C

#include "./headers/initProc.h"


pcb_t* swap_table_pcb;
swap_t swap_pool[2*UPROCMAX];
pcb_t* sst_processes[UPROCMAX];
support_t supports[UPROCMAX];
pcb_t* process_locking_the_swap_mutex = NULL;

// crea un processo sst (il quale a sua volta creerà il processo utente)
void spawn_sst(){
    unsigned int ram_top;
    RAMTOP(ram_top);

    // usata per assegnare gli ASID
    static unsigned int next_asid = 1;

    // l'ASID di questo processo
    unsigned int this_asid = next_asid++;

    // stato dell'SST
    state_t state;
    state.pc_epc = (memaddr)SSTServer;
    state.reg_t9 = (memaddr)SSTServer;
    state.status = IEPON | IMON | TEBITON;
    state.reg_sp = ram_top - (((3 + this_asid) * PAGESIZE));
    state.entry_hi = this_asid << 6;

    // struttura di supporto dell'SST (e del processo figlio)
    supports[this_asid-1].sup_asid = this_asid;
    supports[this_asid-1].sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)pager;
    supports[this_asid-1].sup_exceptContext[PGFAULTEXCEPT].status = IEPON | IMON | TEBITON;
    supports[this_asid-1].sup_exceptContext[PGFAULTEXCEPT].stackPtr = ram_top - (((11 + this_asid) * PAGESIZE));
    supports[this_asid-1].sup_exceptContext[GENERALEXCEPT].pc = (memaddr)GeneralExceptionHandlerSupport;
    supports[this_asid-1].sup_exceptContext[GENERALEXCEPT].status = IEPON | IMON | TEBITON;
    supports[this_asid-1].sup_exceptContext[GENERALEXCEPT].stackPtr = ram_top - (((20 + this_asid) * PAGESIZE));
    for (int i = 0; i < USERPGTBLSIZE-1; i++){
        supports[this_asid-1].sup_privatePgTbl[i].pte_entryHI = ((0x80000+i) << VPNSHIFT) | (this_asid << 6);
        supports[this_asid-1].sup_privatePgTbl[i].pte_entryLO = DIRTYON;
    }
    supports[this_asid-1].sup_privatePgTbl[31].pte_entryHI = 0xBFFFF000 | (this_asid << 6);
    supports[this_asid-1].sup_privatePgTbl[31].pte_entryLO = DIRTYON;

    // richiesta all'SSI
    ssi_create_process_t args;
    args.state = &state;
    args.support = &supports[this_asid-1];

    ssi_payload_t create_process;
    create_process.service_code = CREATEPROCESS;
    create_process.arg = (void*)&args;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&create_process, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&sst_processes[this_asid-1], 0);
}


void test() {

    unsigned int ram_top;
    RAMTOP(ram_top);

    
    // Stato del processo mutex
    state_t mutex_state;
    mutex_state.pc_epc = (memaddr)swapMutexProcess;
    mutex_state.reg_t9 = (memaddr)swapMutexProcess;
    mutex_state.status = IEPON | IMON | TEBITON ;
    mutex_state.reg_sp = ram_top - ((3 * PAGESIZE));

    // richiesta all'SSI
    ssi_create_process_t mutex_args;
    mutex_args.state = &mutex_state;
    mutex_args.support = NULL;

    ssi_payload_t create_mutex;
    create_mutex.service_code = CREATEPROCESS;
    create_mutex.arg = (void*)&mutex_args;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&create_mutex, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&swap_table_pcb, 0);

    // inizializzazione swap_pool
    for (int i = 0; i < 2*UPROCMAX; i++){
        swap_pool[i].sw_asid = -1;
    }

    // inizializzazione SST
    // È possibile sostituire UPROCMAX con un numero più basso a fini di testing
    for (int i = 0; i < UPROCMAX; i++){
        spawn_sst();
    }

    unsigned int r = 0;

    // aspetta che i processi abbiano finito
    for (int i = 0; i < UPROCMAX; i++){
        SYSCALL(RECEIVEMESSAGE, (unsigned int)sst_processes[i], (unsigned int)&r, 0);

        if (r != OK) PANIC();
    }


    // informa l'utente che va tutto bene
    // gli altri test finivano con una citazione a Guida Galattica per Autostoppisti
    // gia che c'ero...
    sst_print_t final_message;
    final_message.length = 65;
    final_message.string = "\"Forty-two,\" said Deep Thought, with infinite majesty and calm.\n";

    writeterminal(&final_message, 1);

    // ti puoi terminare adesso
    ssi_payload_t term;
    term.service_code = TERMPROCESS;
    term.arg = NULL;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&term, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);

}

#endif