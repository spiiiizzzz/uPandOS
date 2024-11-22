#ifndef SSI_H
#define SSI_H

#include "/usr/include/umps3/umps/libumps.h"
#include "../../headers/types.h"
#include "../../headers/const.h"
#include "../../phase1/headers/pcb.h"
#include "syscall.h"


int createProcess(pcb_t* sender, ssi_create_process_t* args);

void terminateProcess(pcb_t* sender, pcb_t* arg);

void doIO(pcb_t* sender, ssi_do_io_t* do_io);

void waitForClock(pcb_t* sender);

int getCPUTime(pcb_t* sender);

int getSupportData(pcb_t* sender);

int getProcessID(pcb_t* sender, void* arg);

void SSIServer();




#endif