#ifndef VMSUPPORT_C
#define VMSUPPORT_C

#include "./headers/vmSupport.h"

extern pcb_t* process_locking_the_swap_mutex;

// Processo che funge da Mutex per la swap pool
void swapMutexProcess(){
    while (TRUE){
        if (process_locking_the_swap_mutex != NULL){ // il mutex è libero
            pcb_t* sender = (pcb_t*)SYSCALL(RECEIVEMESSAGE, (unsigned int)process_locking_the_swap_mutex, (unsigned int)NULL, 0);
            process_locking_the_swap_mutex = NULL;
            SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
        } else {                                     // il mutex è occupato
            pcb_t* sender = (pcb_t*)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)NULL, 0);
            process_locking_the_swap_mutex = sender;
            SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
        }
    }
}

// un algoritmo *estremamente* semplice per scegliere la pagina in cui scrivere
static unsigned int replacementAlgorithm(){
    // controlla se c'è un posto libero
    for (int i = 0; i < 2*UPROCMAX; i++){
        if (swap_pool[i].sw_asid == -1) return i;
    }

    // altrimenti fai una sorta di FIFO
    static unsigned int next = 0;
    return (next++)%(2*UPROCMAX);
}

// funzione per scrivere la pagina nel backing store
static unsigned int writeBackingStore(unsigned int asid, unsigned int block_num, unsigned int ram_block){

    // indirizzo del device
    unsigned int flash_device = (0x100000D4 + ((asid-1) * 0x10));

    ssi_do_io_t do_io = {
        .commandAddr = (unsigned int*)(flash_device + 0x4),
        .commandValue = (block_num << 8) | 3 // scrivi in COMMAND il blocco del flash device
    };

    ssi_payload_t request = {
        .service_code = DOIO,
        .arg = &do_io
    };

    unsigned int response = 0;

    // scrivi in DATA0 l'indirizzo in RAM
    *(unsigned int*)(flash_device + 0x8) = ram_block;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&request, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&response, 0);

    return response;
}

// funzione per leggere la pagina dal backing store
static unsigned int readBackingStore(unsigned int asid, unsigned int block_num, unsigned int ram_block){

    // indirizzo del device
    unsigned int flash_device = (0x100000D4 + ((asid-1) * 0x10));

    ssi_do_io_t do_io = {
        .commandAddr = (unsigned int*)(flash_device + 0x4),
        .commandValue = (block_num << 8) | 2 // scrivi in COMMAND il blocco del flash device
    };

    ssi_payload_t request = {
        .service_code = DOIO,
        .arg = &do_io
    };

    unsigned int response = 0;

    // scrivi in DATA0 l'indirizzo in RAM
    *(unsigned int*)(flash_device + 0x8) = ram_block;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&request, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&response, 0);

    return response;
}

// Il pager del livello supporto
void pager(){

    // richiedi la struttura di supporto
    ssi_payload_t request = {
        .service_code = GETSUPPORTPTR,
        .arg = NULL
    };

    support_t* response = NULL;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&request, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&response, 0);


    // controlla se abbiamo un TLBMOD exception
    // non dovrebbe succedere mai nell'implementazione attuale visto che tutte la pagine sono DIRTY
    // ma non si sa mai
    unsigned int cause = response->sup_exceptState[PGFAULTEXCEPT].cause;

    if ( ((cause << 25) >> 27) == 1){
        TrapExceptionHandlerSupport(response);
    }

    // Ottieni l'accesso esclusivo alla swap pool
    SYSCALL(SENDMESSAGE, (unsigned int)swap_table_pcb, 0, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)swap_table_pcb, 0, 0);

    // Quale pagina dobbiamo scrivere?
    unsigned int missing_page_number = (response->sup_exceptState[PGFAULTEXCEPT].entry_hi - 0x80000000) >> VPNSHIFT;
    if (missing_page_number >= 31 || missing_page_number < 0) missing_page_number = 31; // caso speciale per lo STACK

    // In quale frame dobbiamo riscrivere la pagina?
    unsigned int replacing_frame = replacementAlgorithm();

    // Caso in cui la pagina è già occupata da un'altro processo
    // Aggiorna il TLB e riscrivi la pagina nel backing store
    if (swap_pool[replacing_frame].sw_asid != -1) {
        setSTATUS(getSTATUS() & ~TEBITON);

        swap_pool[replacing_frame].sw_pte->pte_entryLO ^= 0x200;

        setENTRYHI(swap_pool[replacing_frame].sw_pte->pte_entryHI);
        TLBP();
        if ((getINDEX() & 0x80000000) == 0) {
            setENTRYLO(swap_pool[replacing_frame].sw_pte->pte_entryLO);
            TLBWI();
        }
        setSTATUS(getSTATUS() | TEBITON);

        unsigned int r = writeBackingStore(swap_pool[replacing_frame].sw_asid, 
                            (swap_pool[replacing_frame].sw_pte->pte_entryHI - 0x80000000) >> VPNSHIFT, 
                            0x20020000 + replacing_frame*PAGESIZE);
        if (r != 1) TrapExceptionHandlerSupport(response);
    }


    // leggi la nuova pagina dal backing store...
    unsigned int r = readBackingStore(response->sup_asid,
                        missing_page_number,
                        0x20020000 + replacing_frame*PAGESIZE);
    if (r != 1) TrapExceptionHandlerSupport(response);


    // ...e aggiorna la swap pool e il TLB
    setSTATUS(getSTATUS() & ~TEBITON);

    response->sup_privatePgTbl[missing_page_number].pte_entryLO = ((0x20020000 + replacing_frame*PAGESIZE) & 0xFFFFF000) | VALIDON | DIRTYON;

    swap_pool[replacing_frame].sw_pte = &(response->sup_privatePgTbl[missing_page_number]);
    swap_pool[replacing_frame].sw_pageNo = missing_page_number;
    swap_pool[replacing_frame].sw_asid = response->sup_asid;

    setENTRYHI(response->sup_privatePgTbl[missing_page_number].pte_entryHI);
    TLBP();
    if ((getINDEX() & 0x80000000) == 0) {
        setENTRYLO(response->sup_privatePgTbl[missing_page_number].pte_entryLO);
        TLBWI();
    }

    setSTATUS(getSTATUS() | TEBITON);

    // Rilascia l'accesso alla swap pool
    SYSCALL(SENDMESSAGE, (unsigned int)swap_table_pcb, 0, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)swap_table_pcb, 0, 0);

    // finito
    LDST(&(response->sup_exceptState[PGFAULTEXCEPT]));

}

void uTLB_RefillHandler(){
    unsigned int page_number = (((state_t*)BIOSDATAPAGE)->entry_hi >> VPNSHIFT) - (KUSEG >> VPNSHIFT);

    if (page_number >= 31 || page_number < 0) page_number = 31;

    setENTRYHI(current_process->p_supportStruct->sup_privatePgTbl[page_number].pte_entryHI);
    setENTRYLO(current_process->p_supportStruct->sup_privatePgTbl[page_number].pte_entryLO);

    TLBWR();

    LDST(((state_t*)BIOSDATAPAGE));
}

#endif