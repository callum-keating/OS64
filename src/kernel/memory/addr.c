#include "addr.h"
#include "limine/boot_data.h"

uint64_t hhdm_offset = 0;
bitmap_t bitmap = {0};

void generalmm_init() {
    hhdm_offset = boot_data_get_hhdm_response()->offset;
}
