#ifndef SSI_C
#define SSI_C

#include "./headers/ssi.h"


int createProcess(pcb_t* sender, ssi_create_process_t* args){
    pcb_t* to_allocate = allocPcb();
    if (to_allocate == NULL) return NOPROC; // Non vi sono PCB liberi
    // Copia i parametri da args
    to_allocate->p_s.entry_hi = (*(args->state)).entry_hi;
    to_allocate->p_s.cause = (*(args->state)).cause;
    to_allocate->p_s.status = (*(args->state)).status;
    to_allocate->p_s.pc_epc = (*(args->state)).pc_epc;
    for (int i = 0; i < STATE_GPR_LEN; i++){
        to_allocate->p_s.gpr[i] = (*(args->state)).gpr[i];
    }
    to_allocate->p_s.hi = (*(args->state)).hi;
    to_allocate->p_s.lo = (*(args->state)).lo;
    to_allocate->p_supportStruct = args->support;
    to_allocate->p_time = 0;
    // Inserisci il processo nella lista dei figli del sender
    insertChild(sender, to_allocate);

    // Il processo è pronto per l'esecuzione
    insertProcQ(&ready_queue, to_allocate);
    process_count++;

    return (unsigned int)to_allocate;
}

void terminateProcess(pcb_t* sender, pcb_t* arg){
    // Controlla se è stato fornito un parametro
    pcb_t* to_terminate;
    if (arg == NULL) to_terminate = sender;
    else to_terminate = arg;

    // Controlla se il processo effettivamente esiste
    if (ListIsIn(to_terminate, &pcbFree_h)) return;

    // Termina ricorsivamente i figli
    pcb_t* tmp;
    while((tmp = removeChild(to_terminate))){
        terminateProcess(sender, tmp);
    }
    
    // Rimuovi il processo da qualunque lista si trovi
    if (outProcQ(&blocked_pseudo_clock, to_terminate) != NULL){
        soft_blocked_count--;
    } else if (outProcQ(&blocked_msg, to_terminate) != NULL){
        //soft_blocked_count--;
    } else {
        outProcQ(&ready_queue, to_terminate);
    }

    for (int i = 0; i < SEMDEVLEN - 1; i++){
        if (blocked_dev[i] == to_terminate){
            blocked_dev[i] = NULL;
            soft_blocked_count--;
            break;
        }
    }

    // Rimuovi il processo dalla lista dei figli
    outChild(to_terminate);
    // Inserisci il processo nella lista dei processi liberi
    freePcb(to_terminate);

    process_count--;
}

void doIO(pcb_t* sender, ssi_do_io_t* do_io){
    // calcola la entry in cui va inserito il processo
    // versione corretta rispetto a quella di fase 2
    int snum = ((unsigned int)(do_io->commandAddr) - START_DEVREG) / 0x10;
    if ((do_io->commandValue & PRINTCHR) && (snum < 40 && snum >= 32)) { // caso speciale per il subdevice del terminale
        snum += 8;
    }
    blocked_dev[snum] = sender;
    soft_blocked_count++;
    *(do_io->commandAddr) = do_io->commandValue;
}

void waitForClock(pcb_t* sender){
    outProcQ(&blocked_msg, sender);
    insertProcQ(&blocked_pseudo_clock, sender);
    soft_blocked_count++;
}

int getCPUTime(pcb_t* sender){
    return (unsigned int)sender->p_time;
}

int getSupportData(pcb_t* sender){
    return (unsigned int)sender->p_supportStruct;
}

int getProcessID(pcb_t* sender, void* arg){
    if (arg == 0) return sender->p_pid;     // pid del processo
    else if (sender->p_pid == 1) return 0;  // pid del padre (caso speciale per l'SSI)
    return (sender->p_parent)->p_pid;       // pid del padre
}

void SSIServer() {
    while (TRUE) {

        // Ricezione messaggio
        ssi_payload_t* payload;
        pcb_t* sender = (pcb_t*)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)&payload, 0);

            
        // Gestione della richiesta
        unsigned int service = ((ssi_payload_t*)payload)->service_code;
        void* args = ((ssi_payload_t*)payload)->arg;
        unsigned int response = 0;

        switch(service){
            case CREATEPROCESS:
                response = (int)createProcess(sender, (ssi_create_process_t*)args);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)response, 0);
                break;
            case TERMPROCESS:
                terminateProcess(sender, (pcb_t*)args);
                if (args != NULL)
                    SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)OK, 0);
                break;
            case DOIO:
                doIO(sender, (ssi_do_io_t*)args);
                break;
            case GETTIME:
                response = getCPUTime(sender);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)response, 0);
                break;
            case CLOCKWAIT:
                waitForClock(sender);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
                break;
            case GETSUPPORTPTR:
                response = getSupportData(sender);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)response, 0);
                break;
            case GETPROCESSID:
                response = getProcessID(sender, args);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)response, 0);
                break;
            default:
                terminateProcess(sender, NULL);
        }
    }
}




#endif