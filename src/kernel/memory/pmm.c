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
#include <stdint.h>

inline uintptr_t find_bitmap_location(struct limine_memmap_response *memmap) {
    uint64_t largest_free_section = 0;
    uint64_t len_of_largest = 0;

    uint64_t last_free_section = 0;
    uint64_t len_of_last = 0;


    int i;
    for (i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];
        uint64_t current_len = current_entry->length;

        if (current_len > len_of_largest && current_entry->type == LIMINE_MEMMAP_USABLE) {
            len_of_largest = current_len;
            largest_free_section = i;
        }
    }

    if () {
    
    }
}

int init() {
    struct limine_memmap_response *memmap = boot_data_get_memmap_response();
    find_bitmap_location(memmap);
    return 0;
}
