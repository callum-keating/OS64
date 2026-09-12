#include "limine.h"
#include "limine/boot_data.h"
#include "log.h"
#include <stdint.h>
#include <stddef.h>
#define PAGE_SIZE 4096

static uint64_t hhdm_offset = 0;
static struct limine_memmap_response *memmap = NULL;
bool done_init = false;
bool initialised_pages = false;

static uint8_t *page_bitmap;
static uint64_t total_pages;


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
    uint64_t memAmount = 0;
    struct limine_memmap_entry *largest_free_entry = memmap->entries[0];
    for (int i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];

        uint64_t end = current_entry->base + current_entry->length;
        if (end > memAmount)
            memAmount = end;

        if (current_entry->type == LIMINE_MEMMAP_USABLE) {
            logf("found usable memory! base: %p length:%d\n", current_entry->base, current_entry->length);

            if (current_entry->length > largest_free_entry->length) {
                largest_free_entry = current_entry;
            }

        } else if (current_entry->type == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES) {
            logf("Found area where kernel is. this must be recorded and mapped, base: %p length: %d\n", current_entry->base, current_entry->length);
        } else {
            logf("found other memory type. type is %d, base: %p, length: %d\n", current_entry->type, current_entry->base, current_entry->length);
        }
    }
    logf("the largest free entry is of type: %d, its base and lengths are %d and %d\n", largest_free_entry->type, largest_free_entry->base, largest_free_entry->length);
}

uintptr_t alloc_phys_page(void) {

}

void map_page(uintptr_t va, uintptr_t pa, uint64_t flags) {
}

static int setup_blank_pts() {
    traverse_memmap();
}

void setup_pts() {
    if (!done_init) {
        init();
    }
    setup_blank_pts();
    initialised_pages = true;
}
