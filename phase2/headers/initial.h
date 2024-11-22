#ifndef INITIAL_H
#define INITIAL_H


#include "/usr/include/umps3/umps/libumps.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"
#include "../../headers/types.h"
#include "../../headers/const.h"
#include "../../headers/listx.h"
#include "exceptions.h"
#include "ssi.h"
#include "scheduler.h"

// global variables
extern int process_count;
extern int soft_blocked_count;
extern struct list_head ready_queue;
extern pcb_t* current_process;
extern pcb_t* ssi_pcb;
extern pcb_t* blocked_dev[SEMDEVLEN - 1];
extern struct list_head blocked_pseudo_clock;
extern struct list_head blocked_msg;

extern void test();
extern void uTLB_RefillHandler();

void main();

#endif