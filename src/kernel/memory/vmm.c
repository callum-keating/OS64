/*
 * ________________________________________________________________________________________________
 *
 *                                      VIRTUAL MEMORY MANAGER
 * This is the virtual memory manager.
 * It maps and unmaps virtual memory by filling in the Page tables (PML4, PDPT, PD, PT)
 * Since this is a 64 bit system it will use a PML4
 * Current design will not account for userspace and will instead be focused around a single PML4
 * It uses frames outputted by the PMM
 * For the initial write of the page tables the [limine HHDM](https://github.com/limine-bootloader/limine-protocol/blob/trunk/PROTOCOL.md#hhdm-higher-half-direct-map-feature)
 * After the inital paging structure has been made it uses 
 * ________________________________________________________________________________________________
*/
#include "memory/pmm.h"
#include "memory/addr.h"
#include <stdint.h>

static uint64_t (*pml4)[512];

void vmm_init() {
    pml4 = (uint64_t (*)[512])(pmm_alloc_frame() + hhdm_offset);
}
