    .section .text
    .global _start

_start:
    ldr   r0, =_stack_top
    mov   sp, r0

    bl    zero_bss
    bl    main

1:  b     1b

zero_bss:
    ldr   r0, =_bss_start
    ldr   r1, =_bss_end
    movs  r2, #0
0:
    cmp   r0, r1
    bcs   1f
    str   r2, [r0]
    adds  r0, r0, #4
    b     0b
1:
    bx    lr

    .extern main
    .extern _bss_start
    .extern _bss_end
    .extern _stack_top
    
