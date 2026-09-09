#include <stdint.h>
#include <stddef.h>
#include "drivers/serial.h"
#include "limine.h"
#include "limine/limine_requests.h"



// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void boot_data_perform_checks() {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        write_serial_str("Limine base revision not supported. halting");
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        write_serial_str("Limine has not given a framebuffer. halting");
        hcf();
    }

    write_serial_str("Limine checks completed successfully");
};

struct limine_framebuffer_response *boot_data_get_framebuffer_response() {
    return framebuffer_request.response;
}
