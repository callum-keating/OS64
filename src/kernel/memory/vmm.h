#pragma once
#include <stdint.h>

#define VMM_PRESENT     (1ULL << 0)
#define VMM_WRITABLE    (1ULL << 1)
#define VMM_USER        (1ULL << 2)
#define VMM_PWT         (1ULL << 3)
#define VMM_PCD         (1ULL << 4)
#define VMM_ACCESSED    (1ULL << 5)
#define VMM_DIRTY       (1ULL << 6)
#define VMM_HUGE        (1ULL << 7)
#define VMM_GLOBAL      (1ULL << 8)
#define VMM_NOEXEC      (1ULL << 63)

// Software bits
#define VMM_CANFREE     (1ULL << 9)

#define VMM_ADDR_MASK   0x000FFFFFFFFFF000ULL
#define VMM_SW_MASK     (VMM_CANFREE)

#define PML4_INDEX(addr)   (((uint64_t)(addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr)   (((uint64_t)(addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)     (((uint64_t)(addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)     (((uint64_t)(addr) >> 12) & 0x1FF)
#define PAGE_OFFSET(addr)  ((uint64_t)(addr) & 0xFFF)

void     vmm_init(void);
int      vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags);
void     vmm_unmap_page(uint64_t virt);
uint64_t vmm_get_phys(uint64_t virt);
