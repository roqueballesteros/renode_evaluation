// rzt2m_memwatch.c
// Bare-metal "UART + memory watch" demo for Renesas RZ/T2M (dual Cortex-R52) in Renode.

// Minimal stdint
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// === RZ/T2M memory map bits we use ===
// sram0: 0x1000_0000, size 0x0018_0000 (from your REPL)
// We'll watch a word at 0x1000_0200 (feel free to change).
#define WATCH_ADDR   0x10000200UL
volatile uint32_t * const watched = (volatile uint32_t *)WATCH_ADDR;

// === Renesas SCI0 (UART) registers (typical SCI/SCIF-like layout) ===
// Base from your REPL: sci0 @ 0x8000_1000
#define SCI0_BASE   0x80001000UL

// Common SCI register offsets (8-bit registers unless noted)
#define SCSMR   (*(volatile uint8_t  *)(SCI0_BASE + 0x00)) // Serial Mode
#define SCBRR   (*(volatile uint8_t  *)(SCI0_BASE + 0x04)) // Bit Rate
#define SCSCR   (*(volatile uint8_t  *)(SCI0_BASE + 0x08)) // Serial Control
#define SCFTDR  (*(volatile uint8_t  *)(SCI0_BASE + 0x0C)) // Transmit FIFO Data
#define SCFSR   (*(volatile uint16_t *)(SCI0_BASE + 0x10)) // Serial Status (often 16-bit)
#define SCFRDR  (*(volatile uint8_t  *)(SCI0_BASE + 0x14)) // Receive FIFO Data
#define SCFCR   (*(volatile uint16_t *)(SCI0_BASE + 0x18)) // FIFO Control (if present)
#define SCFDR   (*(volatile uint16_t *)(SCI0_BASE + 0x1C)) // FIFO Data Count (if present)

// Bits (typical for Renesas SCI; Renode usually accepts these for simple TX)
#define SCSCR_TE     (1u << 5)      // Transmit Enable
#define SCSCR_RE     (1u << 4)      // Receive Enable (not required here)

#define SCFSR_TDRE   (1u << 7)      // Transmit Data Register Empty (bit 7)
#define SCFSR_TEND   (1u << 6)      // Transmit End (bit 6)

// For 8N1, many Renesas SCI variants use SCSMR=0 for 8 data, 1 stop, no parity.
// We’ll keep it simple: SCSMR = 0.
static void uart_init(void)
{
    // Disable TX/RX before config
    SCSCR = 0x00;

    // 8N1
    SCSMR = 0x00;

    // Baud rate divisor (Renode typically ignores exact timing; pick any sane value)
    SCBRR = 26; // arbitrary placeholder

    // Optionally clear status flags by writing 0 to them (device-specific; harmless in Renode)
    // Many variants clear TDRE/TEND by writing 0 to the corresponding bits.
    SCFSR &= (uint16_t)~(SCFSR_TDRE | SCFSR_TEND);

    // Enable transmitter (and receiver if you want)
    SCSCR = SCSCR_TE; // | SCSCR_RE;
}

static void uart_putc(char c)
{
    // Wait until transmit data register empty
    while((SCFSR & SCFSR_TDRE) == 0) { /* spin */ }

    // Write byte to TX FIFO/Data register
    SCFTDR = (uint8_t)c;

    // Clear TDRE by writing 0 to that bit (common pattern on Renesas SCI)
    SCFSR &= (uint16_t)~SCFSR_TDRE;
}

static void uart_puts(const char *s)
{
    while(*s) {
        if(*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

static void uart_puthex32(uint32_t v)
{
    static const char hx[] = "0123456789ABCDEF";
    for(int i = 28; i >= 0; i -= 4) {
        uart_putc(hx[(v >> i) & 0xF]);
    }
}

static void delay(void)
{
    for(volatile uint32_t i = 0; i < 800000; ++i) { /* crude delay */ }
}

int main(void)
{
    uart_init();

    uart_puts("\n[RZ/T2M] UART up. Memory watch demo.\n");
    uart_puts("Watching 32-bit word at 0x");
    uart_puthex32((uint32_t)WATCH_ADDR);
    uart_puts(" (change it from Renode to see updates)\n");

    uint32_t last = *watched;
    uart_puts("Initial value: 0x");
    uart_puthex32(last);
    uart_puts("\n");

    for(;;) {
        uint32_t v = *watched;
        if(v != last) {
            uart_puts("Change detected @0x");
            uart_puthex32((uint32_t)WATCH_ADDR);
            uart_puts(": 0x");
            uart_puthex32(last);
            uart_puts(" -> 0x");
            uart_puthex32(v);
            uart_puts("\n");
            last = v;
        }
        delay();
    }
}

// Minimal stub for toolchains expecting _exit
void _exit(int status)
{
    (void)status;
    for(;;) { }
}
