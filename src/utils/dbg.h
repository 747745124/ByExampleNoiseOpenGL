#define UNW_LOCAL_ONLY // We only need local unwinder. \
                       // On x86_64 machine, you will need to link with libunwind-x86_64 without this define.

#include "libunwind.h"

#include <stdio.h> // printf

void show_backtrace()
{
    unw_cursor_t cursor;
    unw_context_t uc;
    unw_word_t ip, sp;
    char buf[4096];
    unw_word_t offset;
    unw_getcontext(&uc);          // store registers
    unw_init_local(&cursor, &uc); // initialze with context

    printf("==========\n");

    while (unw_step(&cursor) > 0)
    {                                                     // unwind to older stack frame
        unw_get_reg(&cursor, UNW_REG_IP, &ip);            // read register, rip
        unw_get_reg(&cursor, UNW_REG_SP, &sp);            // read register, rbp
        unw_get_proc_name(&cursor, buf, 4095, &offset);   // get name and offset
        printf("0x%016lx <%s+0x%lx>\n", ip, buf, offset); // x86_64, unw_word_t == uint64_t
    }

    printf("==========\n");
}
