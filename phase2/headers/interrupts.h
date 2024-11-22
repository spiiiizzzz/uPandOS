#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "../../headers/types.h"
#include "../../headers/const.h"
#include "scheduler.h"

cpu_t time();

void InterruptExceptionHandler();

#endif