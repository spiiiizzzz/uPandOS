#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "../../headers/types.h"
#include "../../headers/const.h"
#include "syscall.h"
#include "ssi.h"
#include "interrupts.h"

//sys call exception handler
void SYSCALLExceptionHandler();

//program trap exception handler
void TrapExceptionHandler();

//TLB exception handler
void TLBExceptionHandler();

//general handler
void GeneralExceptionHandler();




#endif


