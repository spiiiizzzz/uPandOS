void SSIServer() {
    while (TRUE) {

        void* payload;
        pcb_t* sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)payload, 0);
        

            
        // Gestione della richiesta
        int service = (ssi_payload_t*)->service;
        void* args = (ssi_payload_t*)->args;
        void* response = NULL;
        switch(service){
            case CREATEPROCESS:
                response = (void*)createProcess(sender, (ssi_create_process_t*)args);
                break;
            case TERMINATEPROCESS:
                terminateProcess(sender, (pcb_t*)args);
                break;
            case DOIO:
                doIO(sender, (ssi_do_io_t*)args);
                break;
            case GETTIME:
                response = (void*)getCPUTime(sender);
                break;
            case CLOCKWAIT:
                waitForClock(sender);
                break;
            case GETSUPPORTPTR:
                response = (void*)getSupportData(sender);
                break;
            case GETPROCESSID:
                response = (void*)getProcessID(sender, args);
        }
        freeMsg(payload);
        

        // Invio dei risultati
        SYSCALL(SENDMESSAGE, (unsigned int)payload->m_sender, (unsigned int)response, 0);


    }
}