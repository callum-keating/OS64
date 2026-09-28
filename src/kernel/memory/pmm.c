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
    uint64_t length;
} bitmap_t;

static bitmap_t bitmap = {0};
struct limine_memmap_response *memmap = {0};
uint64_t hhdm_offset = 0;

inline void find_location_for_bitmap() {
    struct limine_memmap_entry *largest_free_section = NULL;
    uint64_t highest_usable = 0;

    // find the largest usable section and the top of usable memory
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];
        if (current_entry->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t end = current_entry->base + current_entry->length;
        if (end > highest_usable) highest_usable = end;

        if (largest_free_section == NULL || current_entry->length > largest_free_section->length) {
            largest_free_section = current_entry;
        }
    }

    // one bit per frame covering all usable memory, rounded up to whole bytes
    bitmap.length = (highest_usable / PAGE_SIZE + 7) / 8;

    // if the largest free section is too small to fit the bitmap
    if (largest_free_section == NULL || largest_free_section->length < bitmap.length) {
        logf("NOT ENOUGH MEMORY TO STORE BITMAP #STOPPING\n");
        hcf();
    } else {
        logf("enough memory to hold paging bitmap #good\n");
    }

    // set bitmap data
    bitmap.location = (uint64_t *)(largest_free_section->base + hhdm_offset);
}

void fill_bitmap() {
    uint8_t *bits = (uint8_t *)bitmap.location;

    // mark every frame as used first, so gaps and reserved memory are never handed out
    for (uint64_t i = 0; i < bitmap.length; i++) {
        bits[i] = 0xFF;
    }

    // free the frames that limine reported as usable
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *current_entry = memmap->entries[i];
        if (current_entry->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t first = current_entry->base / PAGE_SIZE;
        uint64_t last = (current_entry->base + current_entry->length) / PAGE_SIZE;
        for (uint64_t frame = first; frame < last; frame++) {
            bits[frame / 8] &= (uint8_t)~(1u << (frame % 8));
        }
    }

    // reserve the frames the bitmap itself occupies
    uint64_t bitmap_phys = (uint64_t)bitmap.location - hhdm_offset;
    uint64_t first = bitmap_phys / PAGE_SIZE;
    uint64_t last = (bitmap_phys + bitmap.length + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint64_t frame = first; frame < last; frame++) {
        bits[frame / 8] |= (uint8_t)(1u << (frame % 8));
    }

    logf("I HAVE RUN\n");
}

[[maybe_unused]]
static void log_bitmap() {
    logf("bitmap: ");

    uint8_t *bits = (uint8_t *)bitmap.location;
    uint64_t total_bits = (uint64_t)bitmap.length * 8;

    if (total_bits == 0) {
        logf("\n");
        return;
    }

    int last = bits[0] & 1;
    uint64_t repcount = 0;

    for (uint64_t bit = 0; bit < total_bits; bit++) {
        int current = (bits[bit / 8] >> (bit % 8)) & 1;
        if (current == last) {
            repcount++;
            continue;
        }
        logf("%d*%u  ", last, (unsigned)repcount);
        last = current;
        repcount = 1;
    }

    logf("%d*%u  ", last, (unsigned)repcount);
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
