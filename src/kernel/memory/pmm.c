/*
 * __________________________________________________________________________________________________________________
 *                                  PHYSICAL MEMORY MANAGER
 *
 * Tracks and allocates Physical frames
 * Provides Physical frames to the kernel which is used mostly for the VMM to allocate virtual page tables
 *
 *                                  IMPLEMENTATION SPECIFIC DETAILS
 * This implementation uses 4096 byte (4 KiB) frames
 * This implementation uses a [bitmap page allocator](https://www.amagicsoft.com/wiki/bitmap-allocator.html)
 * It builds the initial list of free frames from limines memory map
 *
 * __________________________________________________________________________________________________________________
*/

#include "limine/boot_data.h"
#include "limine.h"
#include "log.h"
#include "hcf.h"
#include "memory/addr.h"
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t *location;
    // the length is in BYTES
    uint16_t length;
} bitmap_t;

static bitmap_t bitmap = {0};
struct limine_memmap_response *memmap = {0};

inline bitmap_t find_location_for_bitmap() {
    struct limine_memmap_entry *largest_free_section = NULL;
    struct limine_memmap_entry *last_free_section = NULL;


    // find bitmap location
    for (int i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];
        uint64_t current_len = current_entry->length;
        if (largest_free_section == NULL) {
            largest_free_section = current_entry;
        }
        if (current_len > largest_free_section->length && current_entry->type == LIMINE_MEMMAP_USABLE) {
            largest_free_section = current_entry;
        }
        last_free_section = memmap->entries[i];
    }

    // if the largest free section is too small to fit the bitmap
    if (largest_free_section->length < (last_free_section->base + largest_free_section->length) / PAGE_SIZE / 8) {
        logf("largest: %p\tlast: %p\n", largest_free_section, last_free_section);
        hcf();
    } else {
        logf("enough memory to hold paging bitmap #good\n");
    }

    // set bitmap data
    bitmap.location = &largest_free_section->base;
    bitmap.length = (last_free_section->base + largest_free_section->length) / PAGE_SIZE / 8;
}

inline void fill_bitmap() {
    // initially set the bitmap to 0 aka unused
    logf("bitmap: ");
    for (int i = 0; i < bitmap.length; i++) {
        bitmap.location[i] = 0;
        logf("%d", bitmap.location[i]);
    }
    logf("\n");
}

int init() {
    memmap = boot_data_get_memmap_response();
    find_location_for_bitmap();
    fill_bitmap();
    return 0;
}
