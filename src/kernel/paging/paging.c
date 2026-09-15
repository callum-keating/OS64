#include "limine.h"
#include "limine/boot_data.h"
#include "log.h"
#include "memory/string.h"
#include "hcf.h"
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
uint64_t *pml4_virt = NULL;

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

static void fill_memmap_data() {
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

static void init_bitmap() {
    bitmap = (uint64_t *)phys_to_virt(memmap_data.highest_memmap_entry->base);
    memset(bitmap, 0xFF, bitmap_array_size * sizeof(uint64_t));
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            // get base and length and set corresponding bits to zero
            uint64_t first_frame = entry->base / PAGE_SIZE;
            uint64_t last_frame = (entry->base + entry->length - 1) / PAGE_SIZE;
            for (uint64_t frame = first_frame; frame <= last_frame; frame++) {
                bitmap[frame / 64 ] &= ~(1ULL << (frame % 64));
            }
        }
    }
    // make sure the bitmap itself is marked as used
    uint64_t bitmap_phys = memmap_data.highest_memmap_entry->base;
    uint64_t bitmap_pages = (bitmap_array_size * sizeof(uint64_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t bitmap_first_frame = bitmap_phys / PAGE_SIZE;
    for (uint64_t i = 0; i < bitmap_pages; i++) {
        uint64_t frame = bitmap_first_frame + i;

        bitmap[frame / 64] |=
        1ULL << (frame % 64);
    }
}

static void setup_bitmap() {
    fill_memmap_data();
    uint64_t page_count = (memmap_data.memory_len + PAGE_SIZE - 1) / PAGE_SIZE;

    // set to the array count of bitmap
    bitmap_array_size = ((page_count + 7) / 8 + sizeof(uint64_t) - 1) / sizeof(uint64_t);
    uint64_t bitmap_bytes = bitmap_array_size * sizeof(uint64_t);

    // check if there is enough memory to store the bitmap
    if (bitmap_bytes > memmap_data.highest_memmap_entry->length) {
        logf("Not enough memory for paging bitmap. halting\n");
        hcf();
    }
    logf("array size: %d\n", bitmap_array_size);
    init_bitmap();
}

static void mark_used(uint64_t physaddr, uint64_t byte_amount) {
    uint64_t frame_amount =
        (byte_amount + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t first_frame = physaddr / PAGE_SIZE;

    for (uint64_t i = 0; i < frame_amount; i++) {
        uint64_t frame = first_frame + i;
        bitmap[frame / 64] |= 1ULL << (frame % 64);
    }
}


uintptr_t pmm_alloc_page(uint64_t frame, uint64_t virt_addr) {
    // now we need to iterate through the PML4 to see if there is a PDPT

    for (int i = 0; i < 512; i++) {
        
    }

    // if so iterate through PDPT for a PD if not create one and map it

    // find avalible slot in PD (if none then create another PD go recursively up to PML4)

    // once slot is found create PT for frame

    return 0;
}


static inline uint64_t *create_skeleton_pml4() {
    pml4_virt = (bitmap + bitmap_array_size);
    memset(pml4_virt, 0, 512*sizeof(uint64_t));

    // the second param will be one but this is a good example of how to calculate it so i'll keep it
    mark_used(virt_to_phys((uint64_t)pml4_virt), 512 * sizeof(uint64_t));
    return pml4_virt;
}

void map_initial_pages() {
    // this function will create basic page tables that should be similar to the ones already made by limine
    // It will use the HHDM and the bitmap to create pages for each one
    // Framebuffer is mapped with HHDM so it should continue functioning after kernel switches to custom pages
    for (int i = 0; i < bitmap_array_size; i++) {
        if (bitmap[i]) {
            pmm_alloc_page(i);
        }
    }
}

void setup_pts() {
    if (!done_init) {
        init();
    }
    setup_bitmap();
    logf("bitmap: ");
    uint64_t zerocount = 0;
    for (int i = 0; i < bitmap_array_size; i++) {
        for (int b = 0; b < 64; b++) {
            if (((bitmap[i]) >> b) & 1) {
                if (zerocount > 0)
                    logf("(0*%d)", zerocount);
                logf("1");
                zerocount = 0;
                continue;
            }
            zerocount++;
        }
    }
    if (zerocount > 0)
        logf("(0*%d)", zerocount);
    logf("\n");
    // now that the bitmap is created we need the Paging tables
    create_skeleton_pml4();     // create PML4
    map_initial_pages();

    initialised_pages = true;
}
