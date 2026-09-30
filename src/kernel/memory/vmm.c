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
 * Entry 510 will be the recursive entry
 * ________________________________________________________________________________________________
*/
#include "memory/vmm.h"
#include "memory/pmm.h"
#include "memory/addr.h"
#include "log.h"
#include <stdint.h>
#include <stddef.h>

#define REC_PML4  0xFFFFFF7FBFDFE000ULL  /* (R,R,R,R) */
#define REC_PDPT  0xFFFFFF7FBFC00000ULL  /* (R,R,R,·) */
#define REC_PD    0xFFFFFF7F80000000ULL  /* (R,R,·,·) */
#define REC_PT    0xFFFFFF0000000000ULL  /* (R,·,·,·) */

static uint64_t *pml4;

static uint64_t *phys_to_virt(uint64_t phys) {
    return (uint64_t *)(phys + hhdm_offset);
}

static void invlpg(uint64_t virt) {
    asm volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

static void load_cr3(uint64_t phys) {
    asm volatile("mov %0, %%cr3" :: "r"(phys) : "memory");
}

/* Bootstrap only: recursive entry 510 is not installed yet, so tables are
 * dereferenced through the HHDM. */
static uint64_t *get_or_create_table(uint64_t *table, uint16_t index) {
    uint64_t entry = table[index];

    // if entry exists return virtual address
    if (entry & VMM_PRESENT) {
        return phys_to_virt(entry & VMM_ADDR_MASK);
    }

    // otherwise grab a frame and return NULL if there is an issue
    //              such as all memory being used
    uint64_t frame = pmm_alloc_frame();
    if (frame == 0) {
        return NULL;
    }

    // zero newly created table
    uint64_t *new_table = phys_to_virt(frame);
    for (int i = 0; i < 512; i++) {
        new_table[i] = 0;
    }

    // set table to be present and writable
    table[index] = frame | VMM_PRESENT | VMM_WRITABLE;
    return new_table;
}

static uint64_t *vmm_walk(uint64_t virt) {
    // creates all nessasary tables and returns PT
    uint64_t *pdpt = get_or_create_table(pml4, PML4_INDEX(virt));
    if (!pdpt) return NULL;
    uint64_t *pd   = get_or_create_table(pdpt, PDPT_INDEX(virt));
    if (!pd) return NULL;
    return get_or_create_table(pd, PD_INDEX(virt));
}

int vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    if ((virt & 0xFFF) || (phys & 0xFFF)) {
        return -1;                     /* ensures virt and phys are page-aligned */
    }

    uint64_t *pt = vmm_walk(virt);
    if (!pt) {
        return -1;                     /* out of physical frames */
    }

    uint64_t idx = PT_INDEX(virt);
    uint64_t old = pt[idx];

    /* strip VMM_HUGE: on a PTE bit 7 is PAT, not a size bit */
    pt[idx] = (phys & VMM_ADDR_MASK) | (flags & ~VMM_HUGE) | VMM_PRESENT;

    if (old & VMM_PRESENT) {
        invlpg(virt);                  /* replace stale TLB translation */
    }
    return 0;
}

static uint64_t *get_table(uint64_t *table, uint16_t index) {
    uint64_t entry = table[index];
    if (!(entry & VMM_PRESENT)) {
        return NULL;
    }
    return phys_to_virt(entry & VMM_ADDR_MASK);
}

void vmm_unmap_page(uint64_t virt) {
    uint64_t *pdpt = get_table(pml4, PML4_INDEX(virt));
    if (!pdpt) return;

    uint64_t *pd = get_table(pdpt, PDPT_INDEX(virt));
    if (!pd) return;

    uint64_t *pt = get_table(pd, PD_INDEX(virt));
    if (!pt) return;

    uint64_t idx = PT_INDEX(virt);
    if (!(pt[idx] & VMM_PRESENT)) return;

    pt[idx] = 0;
    invlpg(virt);
}

uint64_t vmm_get_phys(uint64_t virt) {
    uint64_t *pdpt = get_table(pml4, PML4_INDEX(virt));
    if (!pdpt) return 0;

    uint64_t *pd = get_table(pdpt, PDPT_INDEX(virt));
    if (!pd) return 0;

    uint64_t *pt = get_table(pd, PD_INDEX(virt));
    if (!pt) return 0;

    uint64_t pte = pt[PT_INDEX(virt)];
    if (!(pte & VMM_PRESENT)) return 0;

    return (pte & VMM_ADDR_MASK) + (virt & 0xFFF);
}

void vmm_init() {
    pml4 = (uint64_t *)(pmm_alloc_frame() + hhdm_offset);
    // zero pml4
    for (int i = 0; i < 512; i++) {
        pml4[i] = 0;
    }
}
