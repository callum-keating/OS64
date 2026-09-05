#include <stdint.h>

void kmain(uint64_t magic, uint64_t multiboot_info)
{
    (void)magic;
    (void)multiboot_info;

    volatile uint16_t *vga = (uint16_t *)0xB8000;

    const char *message = "Hello from OS64!";

    for (int i = 0; message[i] != '\0'; i++) {
        vga[i] = (uint16_t)message[i] | ((uint16_t)0x07 << 8);
    }

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

