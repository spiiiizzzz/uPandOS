#ifndef SYSCALL_C
#define SYSCALL_C

#include "./headers/syscall.h"


// controlla se il processo si trova nella lista
unsigned int ListIsIn(pcb_t *dest, struct list_head *list){
    if (dest == NULL || list == NULL || emptyProcQ(list)) return 0;

    pcb_t *pholder;
    list_for_each_entry(pholder, list, p_list){
        if (pholder==dest){
            return(1);
        }
    }
    return(0);
}


pcb_t* debug_dest;


unsigned int SendMessage(pcb_t *dest, unsigned int payload){

    debug_dest = dest;

    msg_t* to_send = allocMsg();
    to_send->m_sender = current_process;
    to_send->m_payload = payload;

    
    //processo di destinazione esiste
    // il processo sta aspettando un messaggio
    if (ListIsIn(dest, &blocked_msg)) {
        pushMessage(&(dest->msg_inbox), to_send);
        // il processo sta aspettando un messaggio da questo mittente
        if (dest->p_s.reg_a1 == (unsigned int)current_process || dest->p_s.reg_a1 == ANYMESSAGE) { 
            // sveglia il processo
            outProcQ(&blocked_msg, dest);
            insertProcQ(&ready_queue, dest);
            //soft_blocked_count--;
        }
        return(0);
    } else if (ListIsIn(dest, &blocked_pseudo_clock) || ListIsIn(dest, &ready_queue) || dest == current_process){
        pushMessage(&(dest->msg_inbox), to_send);
        return 0;
    } else if (ListIsIn(dest, &pcbFree_h)){  //processo di destinazione in pcbFree_h
        freeMsg(to_send);
        return(DEST_NOT_EXIST);
    }

    freeMsg(to_send);
    return(MSGNOGOOD);
}


unsigned int ReceiveMessage(pcb_t *sender, unsigned int* payload){

    if (sender == ANYMESSAGE) sender = NULL;
    msg_t* tmp = popMessage(&current_process->msg_inbox, sender);
    if (tmp == NULL){

        // salva lo stato del processo
        current_process->p_s.entry_hi = ((state_t*)BIOSDATAPAGE)->entry_hi;
        current_process->p_s.cause = ((state_t*)BIOSDATAPAGE)->cause;
        current_process->p_s.status = ((state_t*)BIOSDATAPAGE)->status;
        current_process->p_s.pc_epc = ((state_t*)BIOSDATAPAGE)->pc_epc;
        for (int i = 0; i < STATE_GPR_LEN; i++){
            current_process->p_s.gpr[i] = ((state_t*)BIOSDATAPAGE)->gpr[i];
        }
        current_process->p_s.hi = ((state_t*)BIOSDATAPAGE)->hi;
        current_process->p_s.lo = ((state_t*)BIOSDATAPAGE)->lo;

        //soft_blocked_count++;

        current_process->p_time += time();

        // blocca il processo
        insertProcQ(&blocked_msg, current_process);
        
        current_process = NULL;
        
        return -1;
    } else {
        if (payload != NULL)
            *payload = tmp->m_payload;
        unsigned int r = (unsigned int)tmp->m_sender;
        freeMsg(tmp);
        return r;
    }
}



unsigned int handleKernelSYSCALL(unsigned int number, unsigned int arg1, unsigned int arg2, unsigned int arg3){

    switch (number){
        case SENDMESSAGE:
            return SendMessage((pcb_t*)arg1, arg2);
        case RECEIVEMESSAGE:
            return ReceiveMessage((pcb_t*)arg1, (unsigned int*)arg2);
    }

    // non dovrebbe mai succedere
    return -1; 
}

#endif