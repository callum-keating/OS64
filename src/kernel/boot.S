.code32

.section .multiboot
.align 8

header_start:
    .long 0xE85250D6
    .long 0
    .long header_end - header_start
    .long -(0xE85250D6 + 0 + (header_end - header_start))

    # Multiboot2 end tag
    .short 0
    .short 0
    .long 8

header_end:


.section .text
.global _start
.type _start, @function

_start:
    cli

    # GRUB gives us:
    # EAX = Multiboot2 magic
    # EBX = Multiboot2 information structure

    movl %eax, multiboot_magic
    movl %ebx, multiboot_info

    # Load GDT
    lgdt gdt64_descriptor

    # Enable PAE
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4

    # Load PML4 into CR3
    movl $page_table_l4, %eax
    movl %eax, %cr3

    # Enable Long Mode
    movl $0xC0000080, %ecx
    rdmsr

    orl $0x100, %eax
    wrmsr

    # Enable paging
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    # Enter 64-bit mode
    ljmp $0x08, $long_mode_start


.code64

long_mode_start:
    # Load data segment
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    # Set up stack
    leaq stack_top(%rip), %rsp

    # Pass Multiboot information to kmain
    #
    # System V x86-64 calling convention:
    # RDI = first argument
    # RSI = second argument

    movl multiboot_magic(%rip), %edi
    movl multiboot_info(%rip), %esi

    call kmain

.hang:
    cli
    hlt
    jmp .hang


# ------------------------------------------------------------
# Page tables
# ------------------------------------------------------------

.section .data
.align 4096

page_table_l4:
    .quad page_table_l3 + 0x003
    .zero 4096 - 8

.align 4096

page_table_l3:
    .quad page_table_l2 + 0x003
    .zero 4096 - 8

.align 4096

page_table_l2:
    # Identity-map the first 1 GiB using 2 MiB pages.
    .set i, 0

    .rept 512
        .quad (i * 0x200000) + 0x083
        .set i, i + 1
    .endr


# ------------------------------------------------------------
# Stack
# ------------------------------------------------------------

.section .bss
.align 16

stack_bottom:
    .skip 16384

stack_top:


# ------------------------------------------------------------
# Multiboot information
# ------------------------------------------------------------

.align 4

multiboot_magic:
    .long 0

multiboot_info:
    .long 0


# ------------------------------------------------------------
# GDT
# ------------------------------------------------------------

.section .data
.align 8

gdt64:
    # Null descriptor
    .quad 0x0000000000000000

    # 64-bit code segment
    .quad 0x00AF9A000000FFFF

    # Data segment
    .quad 0x00AF92000000FFFF

gdt64_end:

gdt64_descriptor:
    .word gdt64_end - gdt64 - 1
    .long gdt64

