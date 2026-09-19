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
