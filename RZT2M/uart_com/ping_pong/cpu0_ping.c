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

#define UART0_BASE 0x80001000UL
#define UART_DR    0x00
#define UART_TDR   0x04
#define UART_FR    0x18
#define FR_TXFF    (1 << 5)
#define FR_RXFE    (1 << 4)

static void uart_putc(uint32_t base, char c)
{
    while (*(volatile uint32_t*)(base + UART_FR) & FR_TXFF);
    *(volatile uint32_t*)(base + UART_TDR) = (uint32_t)c & 0xFF;
}

static void uart_puts(uint32_t base, const char *s)
{
    while (*s) {
        if (*s == '\n') uart_putc(base, '\r');
        uart_putc(base, *s++);
    }
}

static char uart_getc(uint32_t base)
{
    while (*(volatile uint32_t*)(base + UART_FR) & FR_RXFE);
    return (char)(*(volatile uint32_t*)(base + UART_DR) & 0xFF);
}

static void delay(volatile int count)
{
    while(count--);
}

int main(void)
{
    delay(10000); // Let cpu1 boot
    while (1) {
        uart_puts(UART0_BASE, "ping\n");
        char buf[5] = {0};
        for (int i = 0; i < 4; ++i)
            buf[i] = uart_getc(UART0_BASE);
        buf[4] = 0;
        // Optionally, do something with buf (e.g., check if it's "pong")
        delay(10000000);
    }
    return 0;
}

void _exit(int status)
{
    (void)status;
    while (1) { }
}