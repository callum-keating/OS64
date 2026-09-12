#include <stdint.h>
#include <stddef.h>
#include "log.h"
#include "limine.h"
#include "limine/limine_requests.h"
#include "hcf.h"

void boot_data_perform_checks() {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        logf("Limine base revision not supported. halting\n");
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        logf("Limine has not given a framebuffer. halting\n");
        hcf();
    }

    if (rsdp_request.response == NULL) {
        logf("Limine has not given an RSDP. halting\n");
        hcf();
    };

    logf("HHDM offset: %p\n", hhdm_request.response->offset);
    logf("Limine checks completed successfully\n");
};

struct limine_framebuffer_response *boot_data_get_framebuffer_response() {
    return framebuffer_request.response;
}

struct limine_rsdp_response *boot_data_get_rsdp_response() {
    return rsdp_request.response;
};

struct limine_hhdm_response *boot_data_get_hhdm_response() {
    return hhdm_request.response;
};

struct limine_memmap_response *boot_data_get_memmap_response() {
    return memorymap_request.response;
}
