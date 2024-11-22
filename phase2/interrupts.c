#ifndef INTERRUPTS_C
#define INTERRUPTS_C

#include "./headers/interrupts.h"

cpu_t start;

// Calcola il tempo impiegato da un processo
cpu_t time(){
    cpu_t end;
    STCK(end);
    return end-start;
}

// Usata per trovare il numero del device
unsigned int debug_device_line_thing_idk = 0;
static int getDevNum(unsigned int line){
    debug_device_line_thing_idk = *(unsigned int*)(0x10000040+0x4*line) & 0xFF;
    switch (*(unsigned int*)(0x10000040+0x4*line) & 0xFF) {
        case DEV0ON:
            return 0;
        case DEV1ON:
            return 1;
        case DEV2ON:
            return 2;
        case DEV3ON:
            return 3;
        case DEV4ON:
            return 4;
        case DEV5ON:
            return 5;
        case DEV6ON:
            return 6;
        case DEV7ON:
            return 7;
    }
    PANIC();

    // Questo pezzo di codice non verrà mai raggiunto per via del PANIC
    return -1;
}


void InterruptExceptionHandler(){
    unsigned int cause = ((state_t*)BIOSDATAPAGE)->cause;

    if (cause & LOCALTIMERINT){
        setTIMER(TIMESLICE*(*((cpu_t*)TIMESCALEADDR)));

        // Salva lo stato del processo corrente
        current_process->p_s.entry_hi = ((state_t*)BIOSDATAPAGE)->entry_hi;
        current_process->p_s.cause = ((state_t*)BIOSDATAPAGE)->cause;
        current_process->p_s.status = ((state_t*)BIOSDATAPAGE)->status;
        current_process->p_s.pc_epc = ((state_t*)BIOSDATAPAGE)->pc_epc;
        for (int i = 0; i < STATE_GPR_LEN; i++){
            current_process->p_s.gpr[i] = ((state_t*)BIOSDATAPAGE)->gpr[i];
        }
        current_process->p_s.hi = ((state_t*)BIOSDATAPAGE)->hi;
        current_process->p_s.lo = ((state_t*)BIOSDATAPAGE)->lo;

        current_process->p_time += time();

        // Rimuove il processo da current_process e lo inserisce nella ready_queue
        insertProcQ(&ready_queue, current_process);

        current_process = NULL;
    } else if (cause & TIMERINTERRUPT){
        LDIT(PSECOND);
        while (!emptyProcQ(&blocked_pseudo_clock)){
            insertProcQ(&ready_queue, removeProcQ(&blocked_pseudo_clock));
            soft_blocked_count--;
        }
    } else {
        unsigned int device;
        int snum;
        if (cause & DISKINTERRUPT) {
            device = 0x10000054 + (getDevNum(0) * 0x10);
            snum = getDevNum(0);
        } else if (cause & FLASHINTERRUPT) {
            device = 0x10000054 + 0x80 + (getDevNum(1) * 0x10);
            snum = 8 + getDevNum(1);
        } else if (cause & 0x00002000) { // questo interrupt non è segnato in const.h, dovrebbe essere quello dell'ethernet
            device = 0x10000054 + (2 * 0x80) + (getDevNum(2) * 0x10);
            snum = 16 + getDevNum(2);
        } else if (cause & PRINTINTERRUPT) {
            device = 0x10000054 + (3 * 0x80) + (getDevNum(3) * 0x10);
            snum = 24 + getDevNum(3);
        } else if (cause & TERMINTERRUPT) {
            device = 0x10000054 + (4 * 0x80) + (getDevNum(4) * 0x10);
            snum = 32 + getDevNum(4);
            // Controlla il caso particolare per il terminale
            // Se blocked_dev[snum] è NULL allora il processo ha richiesto una print,
            // quindi controlla la posizione più avanti
            if (blocked_dev[snum] == NULL){
                snum += 8;
                device += 0x8;
            }
        }

        // Manda la risposta al processo in attesa per conto dell'SSI
        msg_t* device_response = allocMsg();
        device_response->m_payload = (unsigned int)(*(unsigned int*)device);
        device_response->m_sender = ssi_pcb;
        pcb_t* waiting_process = blocked_dev[snum];
        blocked_dev[snum] = NULL;
        if (waiting_process != NULL){
            pushMessage(&waiting_process->msg_inbox, device_response);
            outProcQ(&blocked_msg, waiting_process);
            insertProcQ(&ready_queue, waiting_process);
            soft_blocked_count--;
        } else {
            freeMsg(device_response);
        }
        // Manda l'acknowledgement al dispositivo di IO
        *(unsigned int*)(device+0x4) = ACK;
    }
}

#endif