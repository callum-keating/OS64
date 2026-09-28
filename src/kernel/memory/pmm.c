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
uint64_t hhdm_offset = 0;

inline void find_location_for_bitmap() {
    struct limine_memmap_entry *largest_free_section = NULL;
    struct limine_memmap_entry *last_free_section = NULL;


    // find bitmap location
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
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
        logf("NOT ENOUGH MEMORY TO STORE BITMAP #STOPPING\n");
        hcf();
    } else {
        logf("enough memory to hold paging bitmap #good\n");
    }

    // set bitmap data
    bitmap.location = (uint64_t *)(largest_free_section->base + hhdm_offset);
    bitmap.length = (largest_free_section->base + hhdm_offset + largest_free_section->length) / PAGE_SIZE / 8;
}

void fill_bitmap() {
    // initially set the bitmap to 0 aka unused
    for (int i = 0; i < bitmap.length; i++) {
        bitmap.location[i] = 0;
    }

    // now set to 1 if the type is LIMINE_MEMMAP_USABLE
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];
        if (current_entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t frame = current_entry->base / PAGE_SIZE;
            uint64_t second = (current_entry->base + current_entry->length) / PAGE_SIZE;
            uint64_t loopnum = second - frame;
            logf("frame = %d\tcurrent_entry->base + current_entry->length = %d\tloopnum = %d\n", frame, second, loopnum);
            for (frame = current_entry->base / PAGE_SIZE; frame < (current_entry->base + current_entry->length) / PAGE_SIZE; frame++) {
                logf("The entry is %d loopnum is %d\n", i, loopnum--);

            }
            logf("LOOPNUM IS NOW AT: %d\n", loopnum);
        }
    }
    logf("I HAVE RUN\n");
}

[[maybe_unused]]
static void log_bitmap() {
    logf("bitmap: ");
    int repcount = 0;
    int last = 2;
    for (int i = 0; i < bitmap.length; i++) {
        for (int offset = 0; offset < 8; offset++) {
            if (last == 2) {
                last = bitmap.location[i];
            }
            if ((bitmap.location[i] & 1 >> offset) == last) {
                repcount++;
                continue;
            }
            logf("%d*%d\n", last,repcount);
            repcount = 0;
            last = bitmap.location[i];
        }
    }
    logf("%d*%d\n", last,repcount);
    logf("\n");
}

int init() {
    memmap = boot_data_get_memmap_response();
    hhdm_offset = boot_data_get_hhdm_response()->offset;
    find_location_for_bitmap();
    fill_bitmap();
    log_bitmap();

    return 0;
}
