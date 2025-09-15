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

// --- GPIO (Renesas_GPIO) ---
#define GPIO_BASE       0x800A0000UL
#define GPIO_PORT_OFS   0x000   // byte per port (8 bits)
#define GPIO_PMODE_OFS  0x200   // 16-bit per port (2 bits per pin)

// Set PortMode PMm[pin] = Output (0b10)
static void gpio_set_mode_output(uint32_t port, uint32_t pin)
{
    volatile uint16_t* pm = (volatile uint16_t*)(GPIO_BASE + GPIO_PMODE_OFS + 2*port);
    uint16_t v = *pm;
    uint32_t shift = (uint32_t)(2 * pin);
    v &= ~(0x3u << shift);
    v |=  (0x2u << shift); // Output
    *pm = v;
}

// Set PortMode PMm[pin] = Input (0b00)
static void gpio_set_mode_input(uint32_t port, uint32_t pin)
{
    volatile uint16_t* pm = (volatile uint16_t*)(GPIO_BASE + GPIO_PMODE_OFS + 2*port);
    uint16_t v = *pm;
    uint32_t shift = (uint32_t)(2 * pin);
    v &= ~(0x3u << shift); // Input
    *pm = v;
}

static void gpio_write(uint32_t port, uint32_t pin, int level)
{
    volatile uint8_t* p = (volatile uint8_t*)(GPIO_BASE + GPIO_PORT_OFS + port);
    uint8_t v = *p;
    if(level) v |=  (uint8_t)(1u << pin);
    else      v &= ~(uint8_t)(1u << pin);
    *p = v;
}

static int gpio_read(uint32_t port, uint32_t pin)
{
    volatile uint8_t* p = (volatile uint8_t*)(GPIO_BASE + GPIO_PORT_OFS + port);
    return ((*p) >> pin) & 0x1;
}

int main(void)
{
    uart_init(UART0_BASE); // Communication UART
    uart_init(UART1_BASE); // Debug UART
    delay(10000); // fast delay for Renode

    // Receive from CPU0 on SCI0
    char msg[64];
    uart_readline(UART0_BASE, msg, (int)sizeof(msg));

    // Print to CPU1 debug (SCI1)
    uart_puts(UART1_BASE, "Hello from CPU1 debug!\n");
    uart_puts(UART1_BASE, "CPU1 received: ");
    uart_puts(UART1_BASE, msg);
    uart_puts(UART1_BASE, "\n");

    // Reply to CPU0 on SCI0
    uart_puts(UART0_BASE, "Hello from CPU1 to CPU0!\n");

    // --- GPIO demo: drive P0.0 HIGH and read P0.1 ---
    gpio_set_mode_output(0, 0);
    gpio_set_mode_input(0, 1);
    gpio_write(0, 0, 1);
    delay(10000); // small delay before reading
    uart_puts(UART1_BASE, "CPU1: set P0.0 HIGH\n");
    uart_puts(UART0_BASE, "CPU1: set P0.0 HIGH\n"); // also print to SCI0

    int in = gpio_read(0, 1);
    uart_puts(UART1_BASE, "CPU1: read P0.1 = ");
    uart_puts(UART1_BASE, in ? "HIGH\n" : "LOW\n");
    uart_puts(UART0_BASE, "CPU1: read P0.1 = ");    // also print to SCI0
    uart_puts(UART0_BASE, in ? "HIGH\n" : "LOW\n");

    // Flush any looped-back SCI1 TX before starting echo
    flush_rx_until_idle(UART1_BASE, 2000);

    // Start echoing terminal input after ~1s (tune the count for your setup)
    echo_debug_input_after_delay(UART1_BASE, 20000000u);

    // unreachable
}

void _exit(int status)
{
    (void)status;
    while (1) { }
}