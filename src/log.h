#ifndef __LOG_H__
#define __LOG_H__

#include "stdafx.h"

// TODO: write a better implementation of this god awful method?
// thinking I could use something like inheritance... although
// i don't want to ruin the structure of this code with classes
// i like the current system i got going on. maybe regular methods
// that take in a file name, line number, message and args? (with macro addition)
#define LOG_MSG(msg) std::cout << msg << std::endl

#endif // __LOG_H__