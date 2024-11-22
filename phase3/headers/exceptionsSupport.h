#ifndef EXCEPTIONS_SUPPORT_H
#define EXCEPTIONS_SUPPORT_H

#include "syscall.h"
#include "vmSupport.h"
#include "/usr/include/umps3/umps/libumps.h"
#include "../../phase2/headers/initial.h"
#include "../../headers/const.h"
#include "../../headers/types.h"

void TrapExceptionHandlerSupport();

void GeneralExceptionHandlerSupport();

#endif