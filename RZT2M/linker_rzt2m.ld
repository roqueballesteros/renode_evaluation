/* RZ/T2M (Cortex-R52) — ATCM for code, SRAM for data */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

/* Memory map (based on Renode’s RZ/T2M description) */
MEMORY
{
  ATCM  (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00080000  /* 512 KB tightly coupled memory for code */
  BTCM  (rwx) : ORIGIN = 0x00100000, LENGTH = 0x00010000  /* 64 KB tightly coupled memory (unused here) */
  SRAM  (rwx) : ORIGIN = 0x10000000, LENGTH = 0x00180000  /* 1.5 MB system RAM for data, bss, stack */
  FLASH (rx)  : ORIGIN = 0x88000000, LENGTH = 0x04000000  /* 64 MB external flash (not used here) */
}

/* Program headers so Renode (and ELF loaders) know segment mapping */
PHDRS
{
  text PT_LOAD FLAGS(5); /* R+X */
  data PT_LOAD FLAGS(6); /* R+W */
}

SECTIONS
{
  /* ------- Code & const data go to ATCM ------- */
  .text : ALIGN(4)
  {
    KEEP(*(.vectors))     /* if a vector table/reset handler is added later */
    *(.text*)             /* all code */
    *(.rodata*)           /* read-only data */
  } > ATCM :text

  /* ------- Initialized data goes to SRAM ------- */
  .data : ALIGN(4)
  {
    _data_start = .;
    *(.data*)
    _data_end = .;
  } > SRAM :data

  /* ------- Zero-initialized data (BSS) in SRAM ------- */
  .bss (NOLOAD) : ALIGN(4)
  {
    _bss_start = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(8);
    _bss_end = .;
  } > SRAM

  /* End-of-image marker */
  _end = .;

  /* ------- Stack pointer symbol at the top of SRAM ------- */
  PROVIDE(_stack_top = ORIGIN(SRAM) + LENGTH(SRAM));
}
