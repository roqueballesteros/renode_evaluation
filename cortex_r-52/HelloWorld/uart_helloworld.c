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

/* Initialize UART0 */
static void uart_init(void)
{
    // Disable UART before configuration
    UART0_CR = 0x0;

    // Set baud rate divisors (dummy values, Renode ignores exact timing)
    UART0_IBRD = 1;
    UART0_FBRD = 0;

    // Set word length = 8 bits, enable FIFO
    UART0_LCRH = LCRH_WLEN_8 | LCRH_FEN;

    // Enable UART, TX and RX
    UART0_CR = CR_UARTEN | CR_TXE | CR_RXE;
}

/* Send one character */
static void uart_putc(char c)
{
    // Wait until TX FIFO is not full
    while (UART0_FR & FR_TXFF);
    UART0_DR = (uint32_t)c;
}

/* Send a null-terminated string */
static void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');   // send CR before LF (for terminals)
        }
        uart_putc(*s++);
    }
}

/* Main entry point */
int main(void)
{
    uart_init();
    uart_puts("\nHello, World!\n");

    while (1) {
        // Stay here forever
    }

    return 0; // not reached
}

/* Minimal stub in case the toolchain expects _exit */
void _exit(int status)
{
    (void)status;
    while (1) { }
}




