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

    while(!emptyChild(to_terminate)){
        terminateProcess(sender, container_of(to_terminate->p_child, pcb_t, p_sib))
    }
    
    if (removeProcQ(blocked_pseudo_clock, to_terminate) != NULL){
        soft_blocked_count--;
    } else {
        for (int i = 0; i < SEMDEVLEN - 1; i++){
            if (removeProcQ(blocked_dev[i], to_terminate) != NULL){
                soft_blocked_count--;
                break;
            }
        }
    }
    outChild(tmp);
    freePcb(tmp);
}


void doIO(pcb_t* sender, ssi_do_io_t* do_io){
    // in theory that's how you get device and interrupt line, there's probably a cleaner way, but maybe I'm just stupid
    // and can't figure it out
    int devNo = (*(do_io->commandAddr) - 0x4 - 0x10000054 & 0x70) >> 4;
    int IntlineNo = (*(do_io->commandAddr) - 0x4 - 0x10000054 & 0x380) >> 7;
    int snum = IntlineNo*8 + devNo;
    // this piece might be wrong
    if (commandAddr & PRINTCHR) {
        snum += 8;
    }

    if (emptyProcQ(blocked_dev[snum])){
        insertProcQ(blocked_dev[snum], sender);
        soft_blocked_count++;
        *(do_io->commandAddr) = do_io->commandValue;
    } else {
        insertProcQ(blocked_dev[snum], sender);
        soft_blocked_count++;
    }
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


