// cpu0.c
#ifndef _STDINT_H
#define _STDINT_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

#endif /* _STDINT_H */

#define UART0_BASE 0x80001000UL  // SCI0

#define UART_DR    0x00  // Receive Data Register (read)
#define UART_TDR   0x04  // Transmit Data Register (write)
#define UART_FR    0x18
#define UART_IBRD  0x24
#define UART_FBRD  0x28
#define UART_LCRH  0x2C
#define UART_CR    0x30

#define FR_TXFF    (1 << 5)
#define CR_UARTEN  (1 << 0)
#define CR_TXE     (1 << 8)
#define CR_RXE     (1 << 9)
#define LCRH_WLEN_8 (3 << 5)
#define LCRH_FEN    (1 << 4)

static void uart_init(uint32_t base)
{
    *(volatile uint32_t*)(base + UART_CR) = 0x0;
    *(volatile uint32_t*)(base + UART_IBRD) = 1;
    *(volatile uint32_t*)(base + UART_FBRD) = 0;
    *(volatile uint32_t*)(base + UART_LCRH) = LCRH_WLEN_8 | LCRH_FEN;
    *(volatile uint32_t*)(base + UART_CR) = CR_UARTEN | CR_TXE | CR_RXE;
}

static void uart_putc(uint32_t base, char c)
{
    while (*(volatile uint32_t*)(base + UART_FR) & FR_TXFF);
    *(volatile uint32_t*)(base + UART_TDR) = (uint32_t)c; // FIX: use TDR (0x04)
}

static void uart_puts(uint32_t base, const char *s)
{
    while (*s) {
        if (*s == '\n') uart_putc(base, '\r');
        uart_putc(base, *s++);
    }
}

static void delay(volatile int count)
{
    while(count--);
}

int main(void)
{
    uart_init(UART0_BASE);
    delay(10000); // Much faster in Renode
    uart_puts(UART0_BASE, "Hello from CPU0 to CPU1!\n");
    while (1) { }
    return 0;
}

void _exit(int status)
{
    (void)status;
    while (1) { }
}