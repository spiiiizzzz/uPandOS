#ifndef EXCEPTIONS_SUPPORT_C
#define EXCEPTIONS_SUPPORT_C

#include "./headers/exceptionsSupport.h"

extern pcb_t* process_locking_the_swap_mutex;

void TrapExceptionHandlerSupport(support_t* support){
    ssi_payload_t request = {
        .service_code = TERMINATE,
        .arg = NULL
    };

    if (process_locking_the_swap_mutex == current_process){
        SYSCALL(SENDMSG, (unsigned int)swap_table_pcb, 0, 0);  
        SYSCALL(RECEIVEMSG, (unsigned int)swap_table_pcb, 0, 0);  
    }

    SYSCALL(SENDMSG, PARENT, (unsigned int)&request, 0);
    SYSCALL(RECEIVEMSG, PARENT, 0, 0);
}

void GeneralExceptionHandlerSupport(){
    ssi_payload_t request = {
        .service_code = GETSUPPORTPTR,
        .arg = NULL
    };

    support_t* response = NULL;

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&request, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&response, 0);

    unsigned int cause = response->sup_exceptState[GENERALEXCEPT].cause;
    cause = ((cause << 25) >> 27); 

    if (cause == 8) syscall_exception_handler(response);
    else TrapExceptionHandlerSupport(response);

    LDST(&(response->sup_exceptState[GENERALEXCEPT]));
}

#endif