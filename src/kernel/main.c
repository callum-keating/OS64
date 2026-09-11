#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "drivers/serial.h"
#include "drivers/pci/pci.h"
#include "limine/boot_data.h"

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    init_serial();
    write_serial_str("Loaded");
    boot_data_perform_checks();

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = boot_data_get_framebuffer_response()->framebuffers[0];

    struct pcieDeviceStruct devices[256] = {0};
    pci_enumeratePci(devices);

    // Print a nice pattern to screen as an example.
    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    volatile uint32_t *fb_ptr = framebuffer->address;
    for (size_t y = 0; y < framebuffer->height; y++) {
        for (size_t x = 0; x < framebuffer->width; x++) {
            uint32_t nX = x * 255 / framebuffer->width;
            uint32_t nY = y * 255 / framebuffer->height;
            fb_ptr[y * (framebuffer->pitch / 4) + x] = (nY << 8) | nX;
        }
    }

    // We're done, just hang...
    hcf();
}
