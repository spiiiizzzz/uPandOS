


//general handler
void GeneralExceptionHandler(){
    memaddr Cause = ((state_t*) BIOSDATAPAGE)->s_cause; 
    int exCode = ((Cause << 25) >> 27); 

    if(exCode == 0) InterruptExceptionHandler();
    else if((exCode <= 3) && (exCode >= 1)) TLBExceptionHandler();
    else if(exCode == 8) SYSCALLExceptionHandler();
    else if (exCode <= 12) TrapExceptionHandler();
}


//pass up or die 

static void PassUp_Or_Die(int index){
    if(currentProcess->p_supportStruct == NULL){
        TerminateProcess(NULL);
        
    }
    else{
       state_t *exceptState = (state_t*) BIOSDATAPAGE;
        
        currentProcess->p_supportStruct->sup_exceptState[index] = *exceptState;

        LDCXT(currentProcess->p_supportStruct->sup_exceptContext[index].c_stackPtr,
                currentProcess->p_supportStruct->sup_exceptContext[index].c_status,
                currentProcess->p_supportStruct->sup_exceptContext[index].c_pc);
    }
}




//sys call exception handler
void SYSCALLExceptionHandler(){
    state_t *exceptState = (state_t*) BIOSDATAPAGE;

    int a0 = exceptState->s_reg[3];  //si può vedere se funziona anche s_a0
        
    if(a0 >= 1) PassUp_Or_Die(GENERALEXCEPT);
}

//program trap exception handler
void TrapExceptionHandler(){
    PassUp_Or_Die(GENERALEXCEPT);
}



//TLB exception handler
void TLBExceptionHandler(){
    PassUp_Or_Die(PGFAULTEXCEPT);
}




