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

#define UART0_BASE 0x80001000UL  // SCI0 (communication)
#define UART1_BASE 0x80001400UL  // SCI1 (debug)

#define UART_DR    0x00
#define UART_TDR   0x04
#define UART_FR    0x18
#define UART_IBRD  0x24
#define UART_FBRD  0x28
#define UART_LCRH  0x2C
#define UART_CR    0x30
#define UART_FRSR  0x50  // FIFOReceiveStatus (Renesas SCI): bit0=DR (data ready)

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
    *(volatile uint32_t*)(base + UART_TDR) = (uint32_t)c;
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

static char uart_getc(uint32_t base)
{
    while((*(volatile uint32_t*)(base + UART_FRSR) & 0x1u) == 0u) { }
    return (char)(*(volatile uint32_t*)(base + UART_DR) & 0xFF);
}

static int uart_readline(uint32_t base, char* buf, int max)
{
    int i = 0;
    char c;
    while(i < max - 1) {
        c = uart_getc(base);
        if(c == '\r') continue;
        if(c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

static int streq(const char* a, const char* b)
{
    while(*a && *b && *a == *b) { a++; b++; }
    return (*a == '\0' && *b == '\0');
}

static void flush_rx_until_idle(uint32_t base, int idleIterations)
{
    int idle = 0;
    while(idle < idleIterations) {
        if((*(volatile uint32_t*)(base + UART_FRSR) & 0x1u) != 0u) {
            (void)(*(volatile uint32_t*)(base + UART_DR)); // drop pending byte
            idle = 0;
        } else {
            idle++;
        }
    }
}

static int strlen_c(const char* s) { int n=0; while(s && *s++) n++; return n; }

static void echo_debug_input_after_delay(uint32_t base, uint32_t delay_cycles)
{
    volatile uint32_t d = delay_cycles; while(d--) { }

    char line[128];
    int idx = 0;
    int lastWasCR = 0;
    int drop_remaining = 0;

    for(;;)
    {
        if(drop_remaining > 0 && ((*(volatile uint32_t*)(base + UART_FRSR) & 0x1u) != 0u)) {
            (void)(*(volatile uint32_t*)(base + UART_DR));
            drop_remaining--;
            continue;
        }

        if((*(volatile uint32_t*)(base + UART_FRSR) & 0x1u) == 0u) {
            continue;
        }

        char c = (char)(*(volatile uint32_t*)(base + UART_DR) & 0xFF);

        if(c == '\r' || c == '\n')
        {
            if(lastWasCR && (c == '\n')) { lastWasCR = 0; continue; }
            line[idx] = '\0';
            if(idx > 0) {
                const char* prefix = "You typed: ";
                uart_puts(base, prefix);
                uart_puts(base, line);
                uart_puts(base, "\n");
                drop_remaining = strlen_c(prefix) + idx + 2;
            }
            idx = 0;
            lastWasCR = (c == '\r');
            continue;
        }

        if(c == 0x08 || c == 0x7F) { if(idx > 0) idx--; lastWasCR = 0; continue; }
        if(idx < (int)sizeof(line) - 1) { line[idx++] = c; line[idx] = '\0'; }
        lastWasCR = 0;
    }
}

int main(void)
{
    uart_init(UART0_BASE); // Communication UART
    uart_init(UART1_BASE); // Debug UART
    
    delay(10000); // fast delay for Renode

    // Send to CPU1 on SCI0
    const char *msg_to_cpu1_line = "Hello from CPU0 to CPU1!";
    const char *msg_to_cpu1 = "Hello from CPU0 to CPU1!\n";
    uart_puts(UART0_BASE, msg_to_cpu1);

    // Receive on SCI0; discard if it's our own line (self-echo), then read the real reply
    char buf[64];
    uart_readline(UART0_BASE, buf, sizeof(buf));
    if(streq(buf, msg_to_cpu1_line)) {
        uart_readline(UART0_BASE, buf, sizeof(buf));
    }

    // Print to CPU0 debug (SCI1)
    uart_puts(UART1_BASE, "Hello from CPU0 to debug!\n");
    uart_puts(UART1_BASE, "CPU0 received: ");
    uart_puts(UART1_BASE, buf);
    uart_puts(UART1_BASE, "\n");

    // Flush any looped-back SCI1 TX before starting echo
    flush_rx_until_idle(UART1_BASE, 2000);

    // Replace the idle loop with the debug echo loop on SCI1
    echo_debug_input_after_delay(UART1_BASE, 20000000u);

    // unreachable
}

void _exit(int status)
{
    (void)status;
    while (1) { }
}
