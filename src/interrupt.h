#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "stdafx.h"

// interrupt id = 0xAA{ID:X2} (0xAA01)
void interrupt_handler(const int interrupt_id);

#endif // __INTERRUPT_H__