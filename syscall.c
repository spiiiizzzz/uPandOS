int SendMessage(pcb_t *dest, unsigned int payload){

    msg_t* to_send = allocMsg();
    to_send->m_sender = current_process;
    to_send->m_payload = payload;

    //processo di destinazione in pcbFree_h
    if(ListIsIn(dest, pcbFree_h)){
        return(DEST_NOT_EXIST);
    }

    //processo di destinazione in coda pronta
    else if(ListIsIn(dest, ready_queue)){
        pushMessage(dest->msg_inbox, to_send);
        return(0);
    }

    //processo in attesa
    else if(ListIsIn(dest, blocked_msg)){
        //TODO: complete
        insertProcQ(destination, puntatore pcbReadyList);
        return(0);
    }

    return(MSGNOGOOD);
}


unsigned int ListIsIn(pcb_t *dest, struct list_head *list){

    struct list_head *pos;

    list_for_each(pos, list) {
        // Confronta l'indirizzo del PCB con quello del processo di destinazione
        // TODO: might be wrong
        if (pos == dest) {
            return 1;
        }
    }
    return 0;
}


int ReceiveMessage(pcb_t *sender, unsigned int payload){

    if (sender == ANYMESSAGE) sender = NULL;
    msg_t* tmp = popMessage(current_process->msg_inbox, sender);
    if (tmp == NULL){
        // bloccati
    } else {
        payload = tmp->m_payload;
        freeMsg(tmp);
        return tmp->m_sender;
    }
}



unsigned int SYSCALL(unsigned int number, unsigned int arg1, unsigned int arg2, unsigned int arg3){

    switch (number){

        case SENDMESSAGE:
            return SendMessage((pcb_t*)arg1, arg);
        case RECEIVEMESSAGE:
            return ReceiveMessage((pcb_t*)arg1, arg2);
    }

}