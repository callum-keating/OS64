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

    if (executable_address_request.response == NULL) {
        logf("Limine has not given executable addresses. halting\n");
        hcf();
    }

    if (paging_mode_request.response == NULL
            || paging_mode_request.response->mode != LIMINE_PAGING_MODE_X86_64_4LVL) {
        logf("Limine is not using 4-level paging. halting\n");
        hcf();
    }

    logf("kernel phys=%p virt=%p\n",
     executable_address_request.response->physical_base,
     executable_address_request.response->virtual_base);

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

struct limine_executable_address_response *boot_data_get_executable_address_response(void) {
    return executable_address_request.response;
}

struct limine_paging_mode_response *boot_data_get_paging_mode_response(void) {
    return paging_mode_request.response;
}
