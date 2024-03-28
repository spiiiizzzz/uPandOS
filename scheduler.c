void scheduler(){
    current_process = removeProcQ(ready_queue);

    if (current_process == NULL){
        if (process_count == 1 && soft_blocked_count == 1){ // non è esattamente quello che c'è scritto nelle specifiche ma dovrebbe essere equivalente
            HALT();
        } else if (process_count > 1 && soft_blocked_count > 1){
            setSTATUS(IECON | IMON);
            setTIMER(99999999); // si può anche disattivare
            WAIT();
        } else if (process_count > 0 && soft_blocked_count == 0){
            PANIC();
        }
    } else {
        setTIMER(PSECOND);
        LDST(current_process -> p_s);
    }
}