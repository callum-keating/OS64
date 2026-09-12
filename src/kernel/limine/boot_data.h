#pragma once
#include "limine.h"

void boot_data_perform_checks();
struct limine_framebuffer_response *boot_data_get_framebuffer_response();
struct limine_rsdp_response *boot_data_get_rsdp_response();
struct limine_hhdm_response *boot_data_get_hhdm_response();
struct limine_memmap_response *boot_data_get_memmap_response();
