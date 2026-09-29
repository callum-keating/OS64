#pragma once
#include <stdint.h>

#define PAGE_SIZE 4096
extern uint64_t hhdm_offset;
void generalmm_init();
typedef struct {
    uint8_t *location;
    // the length is in BYTES
    uint64_t length;
} bitmap_t;

extern bitmap_t bitmap;
