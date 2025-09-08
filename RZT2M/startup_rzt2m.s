    .section .text
    .global _start

/* Minimal reset entry: set SP, clear .bss, call main */
_start:
    ldr   sp, =_stack_top        /* stack grows down from top of SRAM */

    bl    zero_bss               /* clear .bss so globals start at 0 */
    bl    main                   /* jump to C */

1:  b     1b                     /* if main returns, spin forever */

/* void zero_bss(void) */
zero_bss:
    ldr   r0, =_bss_start
    ldr   r1, =_bss_end
    mov   r2, #0
0:
    cmp   r0, r1
    bcs   1f
    str   r2, [r0], #4
    b     0b
1:
    bx    lr
