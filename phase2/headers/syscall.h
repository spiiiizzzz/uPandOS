#ifndef SYSCALL_H
#define SYSCALL_H

#include "../../headers/types.h"
#include "../../headers/const.h"
#include "../../headers/listx.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"
#include "scheduler.h"

extern int process_count;
extern int soft_blocked_count;
extern struct list_head ready_queue;
extern pcb_t* current_process;
extern pcb_t* ssi_pcb;
extern pcb_t* blocked_dev[SEMDEVLEN - 1];
extern struct list_head blocked_pseudo_clock;
extern struct list_head blocked_msg;
extern struct list_head pcbFree_h;
extern pcb_t pcbTable[MAXPROC];

unsigned int ListIsIn(pcb_t *dest, struct list_head *list);

unsigned int SendMessage(pcb_t *dest, unsigned int payload);

unsigned int ReceiveMessage(pcb_t *sender, unsigned int *payload);

unsigned int handleKernelSYSCALL(unsigned int number, unsigned int arg1, unsigned int arg2, unsigned int arg3);

#endif