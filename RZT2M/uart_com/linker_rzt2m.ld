/* RZ/T2M (Cortex-R52) — put everything in SRAM0 so sysbus LoadELF works */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

/* Memory map (from Renode `peripherals`) */
MEMORY
{
  /* ATCM  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00080000  // not used in this variant */
  /* BTCM  (rwx) : ORIGIN = 0x00100000, LENGTH = 0x00010000  // not used in this variant */
  SRAM  (rwx) : ORIGIN = 0x10000000, LENGTH = 0x00180000      /* 1.5 MB */
  /* FLASH (rx)  : ORIGIN = 0x88000000, LENGTH = 0x04000000  // not used here */
}

PHDRS
{
  text PT_LOAD FLAGS(5);  /* R+X */
  data PT_LOAD FLAGS(6);  /* R+W */
}

SECTIONS
{
  /* Code + rodata in SRAM */
  .text : ALIGN(4)
  {
    KEEP(*(.vectors))
    *(.text*)
    *(.rodata*)
  } > SRAM :text

  /* RW data in SRAM */
  .data : ALIGN(4)
  {
    _data_start = .;
    *(.data*)
    _data_end = .;
  } > SRAM :data

  /* BSS in SRAM (zeroed in startup) */
  .bss (NOLOAD) : ALIGN(4)
  {
    _bss_start = .;
    *(.bss*)
    *(COMMON)
    _bss_end = .;
  } > SRAM

  _end = .;

  /* Stack at top of SRAM (symbol only) */
  PROVIDE(_stack_top = ORIGIN(SRAM) + LENGTH(SRAM));
}
