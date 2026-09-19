/*
 * ________________________________________________________________________________________________
 *
 *                                      VIRTUAL MEMORY MANAGER
 * This is the virtual memory manager.
 * It maps and unmaps virtual memory by filling in the Page tables (PML4, PDPT, PD, PT)
 * Since this is a 64 bit system it will use a PML4
 * Current design will not account for userspace and will instead be focused around a single PML4
 * It uses frames outputted by the PMM
 * For the initial write of the page tables the [limine HHDM](https://github.com/limine-bootloader/limine-protocol/blob/trunk/PROTOCOL.md#hhdm-higher-half-direct-map-feature)
 * After the inital paging structure has been made it uses 
 * ________________________________________________________________________________________________
*/
