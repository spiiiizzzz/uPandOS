#ifndef SYSCALLU_H
#define SYSCALLU_H

#include "../../headers/types.h"
#include "../../phase2/headers/initial.h"
#include "/usr/include/umps3/umps/libumps.h"

void syscall_exception_handler(support_t *p_supportStruct);

void send_to_parent(support_t *p_supportStruct, unsigned int payload);

void send_to_process(pcb_t *dest, unsigned int payload);

void receive_from_any(support_t *p_supportStruct, unsigned int payload);

void receive_from_sender(pcb_t *sender, unsigned int payload);

#endif