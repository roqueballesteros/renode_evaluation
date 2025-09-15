/* Cortex-R52 Hello World Linker Script for Renode */

/* Output ELF type */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

/* Define memory regions exactly like Renode */
MEMORY
{
  DRAM  (rwx) : ORIGIN = 0x00000000, LENGTH = 0x10000000  /* 256 MB */
  FLASH (rx)  : ORIGIN = 0x88000000, LENGTH = 0x04000000  /* 64 MB */
}

/* Define program headers */
PHDRS
{
  text PT_LOAD FLAGS(5);   /* 5 = R+X */
  data PT_LOAD FLAGS(6);   /* 6 = R+W */
}

SECTIONS
{
  /* Code + read-only data */
  .text : {
    KEEP(*(.vectors))   /* optional vector table */
    *(.text*)           /* all code */
    *(.rodata*)         /* constants */
  } > DRAM AT > DRAM :text

  /* Initialized data */
  .data : {
    *(.data*)
  } > DRAM AT > DRAM :data

  /* Zero-initialized data (not stored in file) */
  .bss (NOLOAD) : {
    _bss_start = .;
    *(.bss*)
    *(COMMON)
    _bss_end = .;
  } > DRAM

  /* Stack at top of DRAM (let’s reserve 64 KB) */
  .stack (NOLOAD) : {
    . = ALIGN(8);
    _stack_start = .;
    . = . + 0x10000;   /* 64 KB */
    _stack_end = .;
  } > DRAM
}


