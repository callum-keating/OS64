.PHONY: run build clean
export PATH := $(PATH):/usr/sbin:/sbin

# file locations
BUILD := build
BOOTL_BUILD := $(BUILD)/efi
FAT_STAGING_DIR := $(BOOTL_BUILD)/fat
FAT_IMG := $(BUILD)/fat.img
FINAL_IMG := $(BUILD)/final.img

## efi file locations
BOOTL_MAIN := src/bootl/main.c
EFI_PROGRAM_DEST := $(FAT_STAGING_DIR)/EFI/BOOT/BOOTX64.efi
GNU_EFI_DIR := gnu-efi-dir
OVMF_LOCATION := /usr/share/ovmf/OVMF.fd
QEMU_COMMAND := qemu-system-x86_64   -drive format=raw,file=build/final.img   -drive if=pflash,format=raw,readonly=on,file=$(OVMF_LOCATION) -m 512M



# commands and flags

## bootloader commands and flags
BOOTL_CC := clang
BOOTL_CC_FLAGS := -Wall -I$(GNU_EFI_DIR)/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone
BOOTL_LD := ld
BOOTL_LD_FLAGS := -shared -Bsymbolic -L$(GNU_EFI_DIR)/x86_64/lib -L$(GNU_EFI_DIR)/x86_64/gnuefi -T$(GNU_EFI_DIR)/gnuefi/elf_x86_64_efi.lds $(GNU_EFI_DIR)/x86_64/gnuefi/crt0-efi-x86_64.o -lgnuefi -lefi
BOOTL_OBJCOPY := objcopy
BOOTL_OBJCOPY_FLAGS := -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --output-target efi-app-x86_64 --subsystem=10


build: $(FINAL_IMG)

$(FINAL_IMG): $(FAT_IMG)
	dd if=/dev/zero of=$@ bs=1M count=512
	parted $@ --script \
      mklabel gpt \
      mkpart ESP fat32 1MiB 511MiB \
      set 1 esp on
	dd if=build/fat.img of=build/final.img bs=1M seek=1 conv=notrunc


$(FAT_IMG): $(FAT_STAGING_DIR)
	dd if=/dev/zero of=$@ bs=1M count=512
	mkfs.fat -F32 $@
	mcopy -s -i $@ $(FAT_STAGING_DIR)/* ::/

$(FAT_STAGING_DIR): $(EFI_PROGRAM_DEST)


$(EFI_PROGRAM_DEST): $(BOOTL_MAIN)
	@mkdir -p $(dir $@)
	$(BOOTL_CC) $(BOOTL_CC_FLAGS) -c $^ -o $(BOOTL_BUILD)/main.o
	$(BOOTL_LD) $(BOOTL_LD_FLAGS) $(BOOTL_BUILD)/main.o -o $(BOOTL_BUILD)/main.so 
	$(BOOTL_OBJCOPY) $(BOOTL_OBJCOPY_FLAGS) $(BOOTL_BUILD)/main.so $@

run: build
	$(QEMU_COMMAND)

clean:
	rm -rf $(BUILD)
