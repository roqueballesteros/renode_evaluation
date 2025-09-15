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

#define UART0_BASE       0x80001000UL
#define UART_TDR         0x04
#define UART_FR          0x18
#define FR_TXFF          (1 << 5)

#define GPIO_BASE       0x800A0000UL
#define GPIO_PORT_OFS   0x000
#define GPIO_PMODE_OFS  0x200

static void uart_putc(uint32_t base, char c)
{
    while (*(volatile uint32_t*)(base + UART_FR) & FR_TXFF);
    *(volatile uint32_t*)(base + UART_TDR) = (uint32_t)c;
}
static void uart_puts(uint32_t base, const char *s)
{
    while (*s) uart_putc(base, *s++);
}

static void delay(volatile int count)
{
    while(count--);
}

static void gpio_set_mode_output(uint32_t port, uint32_t pin)
{
    volatile uint16_t* pm = (volatile uint16_t*)(GPIO_BASE + GPIO_PMODE_OFS + 2*port);
    uint16_t v = *pm;
    uint32_t shift = (uint32_t)(2 * pin);
    v &= ~(0x3u << shift);
    v |=  (0x2u << shift);
    *pm = v;
}
static void gpio_set_mode_input(uint32_t port, uint32_t pin)
{
    volatile uint16_t* pm = (volatile uint16_t*)(GPIO_BASE + GPIO_PMODE_OFS + 2*port);
    uint16_t v = *pm;
    uint32_t shift = (uint32_t)(2 * pin);
    v &= ~(0x3u << shift);
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
    gpio_set_mode_output(0, 0);
    gpio_set_mode_input(0, 1);
    gpio_write(0, 0, 1);
    delay(10000);

    int in = gpio_read(0, 1);
    uart_puts(UART0_BASE, "CPU0: read P0.1 = ");
    uart_puts(UART0_BASE, in ? "HIGH\n" : "LOW\n");

    while(1) { delay(1000000); }
}

void _exit(int status)
{
    (void)status;
    while (1) { }
}