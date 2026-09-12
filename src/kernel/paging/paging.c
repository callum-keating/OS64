#include "limine.h"
#include "limine/boot_data.h"
#include "log.h"
#include "memory/string.h"
#include <stdint.h>
#include <stddef.h>
#define PAGE_SIZE 4096

struct traversed_data {
    struct limine_memmap_entry *highest_memmap_entry;
    uint64_t memory_len;
};


static uint64_t hhdm_offset = 0;
static struct limine_memmap_response *memmap = NULL;
static struct traversed_data memmap_data = {0};
bool done_init = false;
bool initialised_pages = false;
uint64_t *bitmap = NULL;
uint64_t bitmap_array_size = 0;

static void init() {
    hhdm_offset = boot_data_get_hhdm_response()->offset;
    memmap = boot_data_get_memmap_response();
    done_init = true;
}

uintptr_t phys_to_virt(uint64_t phys) {
    if (!initialised_pages) {   // very simple, just add hhdm_offset
        return phys + hhdm_offset;
    } else {                    // will implement later for now return 0
        return 0;
    }
}

uintptr_t virt_to_phys(uint64_t virt) {
    if (!initialised_pages) {   // very simple, just minus hhdm_offset
        return virt - hhdm_offset;
    } else {                    // will implement later for now return 0
        return 0;
    }
}

static void traverse_memmap() {
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        uint64_t end = entry->base + entry->length;

        logf("memmap %d: base=%x:%x length=%x:%x end=%x:%x type=%d\n",
             (int)i,

             (uint32_t)(entry->base >> 32),
             (uint32_t)entry->base,

             (uint32_t)(entry->length >> 32),
             (uint32_t)entry->length,

             (uint32_t)(end >> 32),
             (uint32_t)end,

             (int)entry->type);

        // check if current memmap entry is the highest
        if (memmap_data.highest_memmap_entry == NULL) {
            if (entry->type == LIMINE_MEMMAP_USABLE)
                memmap_data.highest_memmap_entry = entry;
            continue;
        }
        if (entry->base + entry->length > memmap_data.highest_memmap_entry->base + memmap_data.highest_memmap_entry->length && entry->type == LIMINE_MEMMAP_USABLE) {
            memmap_data.highest_memmap_entry = entry;
        }
    }
    memmap_data.memory_len = memmap_data.highest_memmap_entry->base + memmap_data.highest_memmap_entry->length;
}

static void setup_bitmap() {
    traverse_memmap();
    uint64_t page_count = (memmap_data.memory_len + PAGE_SIZE - 1) / PAGE_SIZE;

    // set to the array count of bitmap
    bitmap_array_size = ((page_count + 7) / 8 + sizeof(uint64_t) - 1) / sizeof(uint64_t);
    logf("array size: %d\n", bitmap_array_size);
    bitmap = (uint64_t *)phys_to_virt(memmap_data.highest_memmap_entry->base);
    memset(bitmap, 0xFF, bitmap_array_size * sizeof(uint64_t));
}

void setup_pts() {
    if (!done_init) {
        init();
    }
    setup_bitmap();
    initialised_pages = true;
}
