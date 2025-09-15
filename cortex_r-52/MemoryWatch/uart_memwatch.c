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

/* UART0 base address from Renode's Cortex-R52 platform */
#define UART0_BASE 0x9C090000UL
#define UART0_DR   (*(volatile uint32_t*)(UART0_BASE + 0x00))  // Data Register
#define UART0_FR   (*(volatile uint32_t*)(UART0_BASE + 0x18))  // Flag Register
#define UART0_IBRD (*(volatile uint32_t*)(UART0_BASE + 0x24))  // Integer Baud Rate Divisor
#define UART0_FBRD (*(volatile uint32_t*)(UART0_BASE + 0x28))  // Fractional Baud Rate Divisor
#define UART0_LCRH (*(volatile uint32_t*)(UART0_BASE + 0x2C))  // Line Control
#define UART0_CR   (*(volatile uint32_t*)(UART0_BASE + 0x30))  // Control

/* Flag Register bits */
#define FR_TXFF    (1 << 5)  // Transmit FIFO Full

/* Control Register bits */
#define CR_UARTEN  (1 << 0)  // UART Enable
#define CR_TXE     (1 << 8)  // Transmit Enable
#define CR_RXE     (1 << 9)  // Receive Enable

/* Line Control bits */
#define LCRH_WLEN_8 (3 << 5) // Word length = 8 bits
#define LCRH_FEN    (1 << 4) // Enable FIFOs

/* RAM address we want to monitor */
#define WATCH_ADDR 0x2000
volatile uint32_t *watched = (volatile uint32_t *)WATCH_ADDR;

/* Initialize UART0 */
static void uart_init(void)
{
    UART0_CR = 0x0;                // Disable UART
    UART0_IBRD = 1;                // Dummy values
    UART0_FBRD = 0;
    UART0_LCRH = LCRH_WLEN_8 | LCRH_FEN;
    UART0_CR = CR_UARTEN | CR_TXE | CR_RXE; // Enable UART
}

/* Send one character */
static void uart_putc(char c)
{
    while (UART0_FR & FR_TXFF);
    UART0_DR = (uint32_t)c;
}

/* Send a null-terminated string */
static void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}

/* Convert 32-bit value to hex string */
static void uart_puthex(uint32_t val)
{
    const char hex[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        uart_putc(hex[(val >> i) & 0xF]);
    }
}

/* Simple delay */
static void delay(void)
{
    for (volatile int i = 0; i < 1000000; i++);
}

/* Main entry point */
int main(void)
{
    uart_init();
    uart_puts("\nHello, World!\n");
    uart_puts("Watching memory at 0x2000...\n");

    while (1) {
        uint32_t val = *watched;
        uart_puts("Value at 0x2000 = 0x");
        uart_puthex(val);
        uart_puts("\n");
        delay();
    }

    return 0; // not reached
}

/* Minimal stub in case the toolchain expects _exit */
void _exit(int status)
{
    (void)status;
    while (1) { }
}
