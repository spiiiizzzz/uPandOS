pcb_t* createProcess(pcb_t* sender, ssi_create_process_t* args){
    pcb_t* to_allocate = allocPcb();
    if (to_allocate == NULL) return NOPROC;

    to_allocate->p_s = arg->state;
    to_allocate->p_supportStruct = arg->support;
    insertProcQ(ready_queue.p_list, to_allocate);
    insertChild(sender, to_allocate);
    to_allocate->p_time = 0;

    process_count++;

    return to_allocate
}


void terminateProcess(pcb_t* sender, pcb_t* arg){
    pcb_t* to_terminate;
    if (arg == NULL) to_terminate = sender;
    else to_terminate = arg;

    while(!emptyChild(arg)){
        terminateProcess(sender, container_of(arg->p_child, pcb_t, p_sib))
    }
    
    // TODO: cleanup stuff explained in section 11
    outChild(tmp);
    freePcb(tmp);
}


void doIO(pcb_t* sender, ssi_do_io_t* do_io){

    *(do_io->commandAddr) = do_io->commandValue;

}

void doIOResponse(){
}

void waitForClock(pcb_t* sender){
    insertProcQ(blocked_pseudo_clock, sender);
    soft_blocked_count++;
}

unsigned int getCPUTime(pcb_t* sender){
    return sender->p_time;
}

p_supportStruct* getSupportData(pcb_t* sender){
    return sender->p_supportStruct;
}

int getProcessID(pcb_t* sender, void* arg){ // not sure what the arg is supposed to be
    if (arg == 0) return sender->p_pid;
    else if (sender->p_pid == 1) return 0;
    return (sender->p_parent)->p_pid;
}


