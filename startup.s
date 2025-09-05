        .section .text
    .global _start

_start:
    ldr sp, =_stack_end   /* setup stack pointer */
    bl main               /* call main */
1:  b 1b                  /* infinite loop if main returns */


