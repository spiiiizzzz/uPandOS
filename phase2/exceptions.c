#ifndef EXCEPTIONS_C
#define EXCEPTIONS_C

#include "./headers/exceptions.h"

void PassUp_Or_Die(int index){
    if(current_process->p_supportStruct == NULL){
        terminateProcess(current_process, NULL);
        current_process = NULL;
    }
    else{
        state_t *exceptState = (state_t*) BIOSDATAPAGE;

        current_process->p_supportStruct->sup_exceptState[index].entry_hi = exceptState->entry_hi;
        current_process->p_supportStruct->sup_exceptState[index].cause = exceptState->cause;
        current_process->p_supportStruct->sup_exceptState[index].status = exceptState->status;
        current_process->p_supportStruct->sup_exceptState[index].pc_epc = exceptState->pc_epc;
        for (int i = 0; i < STATE_GPR_LEN; i++){
            current_process->p_supportStruct->sup_exceptState[index].gpr[i] = exceptState->gpr[i];
        }
        current_process->p_supportStruct->sup_exceptState[index].hi = exceptState->hi;
        current_process->p_supportStruct->sup_exceptState[index].lo = exceptState->lo;

        LDCXT(current_process->p_supportStruct->sup_exceptContext[index].stackPtr,
                current_process->p_supportStruct->sup_exceptContext[index].status,
                current_process->p_supportStruct->sup_exceptContext[index].pc);
    }

}


//sys call exception handler
void SYSCALLExceptionHandler(){
    state_t *exceptState = (state_t*) BIOSDATAPAGE;

    int a0 = exceptState->reg_a0;
        
    if(a0 >= 1) PassUp_Or_Die(GENERALEXCEPT);
    else {
        unsigned int v0 = handleKernelSYSCALL(exceptState->reg_a0, exceptState->reg_a1, exceptState->reg_a2, exceptState->reg_a3);
        if (v0 != -1){
            exceptState->pc_epc += 0x4;
            exceptState->reg_v0 = v0;
        }
    }
}

//program trap exception handler
void TrapExceptionHandler(){
    PassUp_Or_Die(GENERALEXCEPT);
}



//TLB exception handler
void TLBExceptionHandler(){
    PassUp_Or_Die(PGFAULTEXCEPT);
}


//general handler
void GeneralExceptionHandler(){
    // Disabilita gli interrupt
    setSTATUS(getSTATUS() & ~TEBITON);

    memaddr Cause = ((state_t*) BIOSDATAPAGE)->cause; 
    int exCode = ((Cause << 25) >> 27); 

    if(exCode == 0) InterruptExceptionHandler();
    else if((exCode <= 3) && (exCode >= 1)) TLBExceptionHandler();
    else if(exCode == 8) SYSCALLExceptionHandler();
    else if (exCode <= 12) TrapExceptionHandler();

    // Riabilita gli interrupt
    setSTATUS(getSTATUS() | TEBITON);

    if (current_process != NULL) LDST((state_t*)BIOSDATAPAGE);
    else scheduler();
}



#endif


