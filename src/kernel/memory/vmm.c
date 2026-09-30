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
#include "limine/boot_data.h"
#include "log.h"
#include "hcf.h"
#include <stdint.h>
#include <stddef.h>

#define REC_PML4  0xFFFFFF7FBFDFE000ULL  /* (R,R,R,R) */
#define REC_PDPT  0xFFFFFF7FBFC00000ULL  /* (R,R,R,·) */
#define REC_PD    0xFFFFFF7F80000000ULL  /* (R,R,·,·) */
#define REC_PT    0xFFFFFF0000000000ULL  /* (R,·,·,·) */

static uint64_t *pml4;

extern uint8_t __kernel_start[];
extern uint8_t __kernel_end[];
extern uint8_t __text_start[];
extern uint8_t __text_end[];
extern uint8_t __rodata_start[];
extern uint8_t __rodata_end[];
extern uint8_t __data_start[];
extern uint8_t __data_end[];
extern uint8_t __bss_start[];
extern uint8_t __bss_end[];

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

#define VMM_TEST_VA 0xffffffffc0000000ULL

void vmm_init(void) {
    uint64_t pml4_phys = pmm_alloc_frame();
    if (pml4_phys == 0) {
        logf("vmm: out of frames allocating PML4\n");
        hcf();
    }

    pml4 = phys_to_virt(pml4_phys);
    for (int i = 0; i < 512; i++) {
        pml4[i] = 0;
    }

    /* Map the HHDM (and everything else Limine described) with 4 KiB pages so
     * every physical frame stays reachable after we load our own CR3. */
    struct limine_memmap_response *memmap = boot_data_get_memmap_response();
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *e = memmap->entries[i];
        uint64_t start = e->base & ~0xFFFULL;
        uint64_t end   = (e->base + e->length + 0xFFF) & ~0xFFFULL;
        for (uint64_t phys = start; phys < end; phys += PAGE_SIZE) {
            vmm_map_page(phys + hhdm_offset, phys, VMM_PRESENT | VMM_WRITABLE);
        }
    }

    /* Map the kernel image: the virtual range is fixed by the linker, the
     * physical location comes from Limine. */
    struct limine_executable_address_response *exec =
        boot_data_get_executable_address_response();
    for (uint64_t virt = (uint64_t)__kernel_start;
         virt < (uint64_t)__kernel_end; virt += PAGE_SIZE) {
        uint64_t phys = exec->physical_base + (virt - exec->virtual_base);
        vmm_map_page(virt, phys, VMM_PRESENT | VMM_WRITABLE);
    }

    /* Optional recursive entry 510 (the public API still walks via the HHDM). */
    pml4[510] = pml4_phys | VMM_PRESENT | VMM_WRITABLE;

    /* Self-test: map a scratch frame, read the translation back, then unmap. */
    if (PML4_INDEX(0xffffffff80000000ULL) != 511) {
        logf("vmm: kernel is not in PML4 entry 511\n");
        hcf();
    }

    uint64_t test_frame = pmm_alloc_frame();
    if (test_frame == 0
     || vmm_map_page(VMM_TEST_VA, test_frame, VMM_PRESENT | VMM_WRITABLE) != 0
     || vmm_get_phys(VMM_TEST_VA) != test_frame) {
        logf("vmm: self-test mapping failed\n");
        hcf();
    }

    vmm_unmap_page(VMM_TEST_VA);
    if (vmm_get_phys(VMM_TEST_VA) != 0) {
        logf("vmm: self-test unmap failed\n");
        hcf();
    }

    logf("vmm: page tables built, loading CR3\n");

    load_cr3(pml4_phys);
    logf("vmm: new page tables active\n");
}
