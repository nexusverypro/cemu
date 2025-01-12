#include "interrupt.h"
#include "log.h"

void interrupt_handler(int interrupt_id) {
    switch (interrupt_id) {
        case 0xAA01: {
            LOG_MSG("[+] Interrupt 0xAA01");
            break;
        }
        default: {
            LOG_MSG("[!] Unknown interrupt: 0x" << std::hex << interrupt_id);
            break;
        }
    }
}