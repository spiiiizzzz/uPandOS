#ifndef SCHEDULER_C
#define SCHEDULER_C

#include "./headers/scheduler.h"


void scheduler(){
	// Usato in seguito per calcolare il tempo occupato dal processo
	STCK(start);

	if(emptyProcQ(&ready_queue)){
		if(process_count == 1)
			/*SSI unico processo nel sistema */
			HALT(); /* HALT BIOS service/instruction */
			
		if(process_count > 1 && soft_blocked_count > 0){
			current_process = NULL;
			/* Attivare tutti gli interrupts e disabilitare PLT */
			setTIMER(0xFFFFFFFF);
			setSTATUS(ALLOFF | IMON | IECON);
			WAIT(); /*entra in Wait State */
		}
		if(process_count > 0 && soft_blocked_count == 0)
			/* deadlock */
			PANIC(); /* PANIC BIOS service/instruction*/
	} else {
		
		current_process = removeProcQ(&ready_queue);
		/* carica il timeslice round-robin nel timer locale del processore */
		setTIMER(TIMESLICE*(*((cpu_t*)TIMESCALEADDR)));
		/* carica lo stato nel processore del processo corrente */
		LDST(&current_process->p_s);
	}
}

#endif