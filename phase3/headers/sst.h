#ifndef SST_H
#define SST_H

#include "../../headers/types.h"
#include "../../headers/const.h"
#include "../../phase2/headers/initial.h"
#include "/usr/include/umps3/umps/libumps.h"
#include "vmSupport.h"
#include "exceptionsSupport.h"


int gettod();

void terminate(pcb_t* sender, ssi_payload_t* payload);

void writeprinter(sst_print_t* args, unsigned int asid);

void writeterminal(sst_print_t* args, unsigned int asid);

void SSTServer();

#endif