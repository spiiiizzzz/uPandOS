#ifndef SST_C
#define SST_C

#include "./headers/sst.h"

extern pcb_t* swap_table_pcb;
extern swap_t swap_pool[];

/* System service thread */
#define GET_TOD			   1
#define TERMINATE		   2
#define WRITEPRINTER	   3
#define WRITETERMINAL 	   4
#define EOS '\0'

typedef unsigned int devregtr;

#define TERM0ADDR 0x10000254
#define TERMSTATMASK 0xFF

int gettod(){
    cpu_t tod;
    STCK(tod);
    return(tod);
    
    //il tempo deve essere mandato come payload di messaggio al processo figlio 
}

void terminate(pcb_t* sender, ssi_payload_t* payload){

    SYSCALL(SENDMESSAGE, (unsigned int)current_process->p_parent, OK, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)current_process->p_parent, 0, 0);
    
    payload->service_code = TERMPROCESS ; // non per forza necessario visto che il valore precedente, ovvere TERMINATE = 2 = TERMINATEPROCESS
    payload->arg = NULL;
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, 0, 0);
}




void writeprinter(sst_print_t* args, unsigned int asid){
    //manda un messaggio alla ssi per doio sapendo che ogni u-proc ha una sua unica stampante ASID
    //nel payload è presente il codice della operazione, e la strutture print_payload_t che a sua volta contiene la lunghezza della stringa e la stringa stessa.
    unsigned int s = args->length;
    char* stringa = args->string;


    devregtr *base = (devregtr*)( 0x100001D4 + ((asid-1) * 0x10));
    devregtr *command = base + 1;
    devregtr status;


    for ( int i = 0; (stringa[i] != EOS) && (i < s) ; i++){
        devregtr value = 2;
        ssi_do_io_t do_io = {
            .commandAddr = command,
            .commandValue = value,
        };
        ssi_payload_t payload = {
            .service_code = DOIO,
            .arg = &do_io,
        };

        *(base + 2) = ((unsigned int)(stringa[i]));

        SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
        SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);

        if ((status & TERMSTATMASK) != 1)
            PANIC();
    }



}


void writeterminal(sst_print_t* args, unsigned int asid){
    //manda un messaggio alla ssi per doio sapendo che ogni u-proc ha un suo unico terminale che coincide con il sui ASID
    //nel payload è presente il codice della operazione, e la strutture print_payload_t che a sua volta contiene la lunghezza della stringa e la stringa stessa.
    unsigned int s = args->length;
    char* stringa = args->string;


    devregtr *base = (devregtr *)( 0x10000254 + ((asid-1) * 0x10));
    devregtr *command = base + 3;
    devregtr status;


    for ( int i = 0; (stringa[i] != EOS) && (i < s) ; i++){
        devregtr value = PRINTCHR | (((devregtr)stringa[i]) << 8);
        ssi_do_io_t do_io = {
            .commandAddr = command,
            .commandValue = value,
        };
        ssi_payload_t payload = {
            .service_code = DOIO,
            .arg = &do_io,
        };
        SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
        SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);

        if ((status & TERMSTATMASK) != RECVD)
            PANIC();
    }


}



void SSTServer(){

    // richiedo il support struct
    ssi_payload_t request_support;
    request_support.service_code = GETSUPPORTPTR;
    request_support.arg = NULL;

    support_t* support = NULL;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&request_support, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&support, 0);

    // creo il processo utente
    state_t state;
    state.pc_epc = 0x800000B0;
    state.reg_t9 = 0x800000B0;
    state.reg_sp = 0xC0000000;
    state.status = USERPON | TEBITON | IMON | IEPON;
    state.entry_hi = support->sup_asid << 6;

    ssi_create_process_t process;
    process.state = &state;
    process.support = support;

    ssi_payload_t payload;
    payload.service_code = CREATEPROCESS;
    payload.arg = &process;

    pcb_t* child = NULL;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&child, 0);

    while (TRUE) {

        ssi_payload_t* payload;
        pcb_t* sender = (pcb_t*)SYSCALL(RECEIVEMESSAGE, (unsigned int)child , (unsigned int)&payload, 0);
        
            
        // Gestione della richiesta

        unsigned int service = ((ssi_payload_t*)payload)->service_code;
        void* args = ((ssi_payload_t*)payload)->arg;
        unsigned int response = 0;


        switch(service){
            case GET_TOD:
                response = (unsigned int)gettod(sender);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)response, 0);
                break;
            case TERMINATE:
                terminate(sender, payload);
                break;
            case WRITEPRINTER:
                writeprinter((sst_print_t*)args, support->sup_asid);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
                break;
            case WRITETERMINAL:
                writeterminal((sst_print_t*)args, support->sup_asid);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
                break;
        }
    }
}



#endif

