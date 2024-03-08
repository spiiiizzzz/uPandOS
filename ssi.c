struct ssi_request_and_payload {
    int service;
    void* arg
};


void SSIRequest(pcb_t* sender, int service, void* arg) {
    struct ssi_request_and_payload* args = {
        service,
        arg
    } ;
    msg_t* payload = allocMsg();
    payload->m_sender = sender;
    payload->m_payload = args;
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_process, (unsigned int)payload, 0);
}


void SSIServer() {
    while (TRUE) {

        msg_t* payload;
        SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)payload, 0);
        

            
        // Gestione della richiesta
        int service = (payload->m_payload).service;
        void* args = (payload->m_payload).args;
        void* response;
        switch(service){
            case CREATEPROCESS:
                response = (void*)createProcess(payload->m_sender, (ssi_create_process_t*)args);
                break;
            case TERMINATEPROCESS:
                terminateProcess(payload->m_sender, (pcb_t*)args);
                break;
            case DOIO:
                break;
            case GETTIME:
                response = (void*)getCPUTime(payload -> m_sender);
                break;
            case CLOCKWAIT:
                waitForClock(payload -> m_sender);
                break;
            case GETSUPPORTPTR:
                response = (void*)getSupportData(payload->m_sender);
                break;
            case GETPROCESSID:
                response = (void*)getProcessID(payload -> m_sender, args);
        }
        freeMsg(payload);
        

        // Invio dei risultati
        msg_t* payload = allocMsg();
        payload->m_sender = ssi_process;
        payload->m_payload = response;
        SYSCALL(SENDMESSAGE, (unsigned int)payload->m_sender, (unsigned int)payload, 0);


    }
}