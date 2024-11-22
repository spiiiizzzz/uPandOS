#ifndef SYSCALLU_C
#define SYSCALLU_C

#include "./headers/syscall.h"


void syscall_exception_handler(support_t *p_supportStruct){

    unsigned int syscall_num = p_supportStruct->sup_exceptState[GENERALEXCEPT].reg_a0;
    unsigned int a1 = p_supportStruct->sup_exceptState[GENERALEXCEPT].reg_a1;
    unsigned int a2 = p_supportStruct->sup_exceptState[GENERALEXCEPT].reg_a2;
    unsigned int *v0 = &(p_supportStruct->sup_exceptState[GENERALEXCEPT].reg_v0);

    switch(syscall_num){ 

        case SENDMSG:

            if(a1==PARENT){
                //funzione invio messaggio al processo genitore
                *v0 = SYSCALL(SENDMESSAGE, (unsigned int)current_process->p_parent, a2, 0);
            }else{
                //funzione invio messaggio al processo specificato 
                *v0 = SYSCALL(SENDMESSAGE, a1, a2, 0);
            }
            p_supportStruct->sup_exceptState[GENERALEXCEPT].pc_epc += 4;
            break;


        case RECEIVEMSG:

            *v0 = SYSCALL(RECEIVEMESSAGE, a1, a2, 0);
            if (*v0 != -1)
                p_supportStruct->sup_exceptState[GENERALEXCEPT].pc_epc += 4;
            break;


        default:
            *v0 = -1; //fallimento
            p_supportStruct->sup_exceptState[GENERALEXCEPT].pc_epc += 4;
            break;
    }

}
#endif