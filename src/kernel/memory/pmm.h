#include <stdint.h>

int pmm_init();
uint64_t pmm_alloc_frame();
void pmm_free_frame(uint64_t phys);
