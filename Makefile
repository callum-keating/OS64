.PHONY: build run clean


CC 			:= clang
CFLAGS 		:= --target=x86_64-elf \
    -Wall \
    -Wextra \
	-std=gnu23 \
	-O2 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -mcmodel=kernel \
	-Isrc/kernel/inc

LD 			:= ld
LDFLAGS 	:= -m elf_x86_64

ASM			:= clang
ASMFLAGS 	:= --target=x86_64-elf


# locations
SRC_DIR		:= src
BUILD_DIR	:= build
OBJ_DIR		:= $(BUILD_DIR)/objs
FS_DIR		:= $(BUILD_DIR)/fs
KERNEL_ELF	:= $(FS_DIR)/boot/kernel.elf
FINAL_IMG	:= $(BUILD_DIR)/final.img

# kernel files
KERNEL_SRC_DIR		:= $(SRC_DIR)/kernel
KERNEL_BUILD_DIR	:= $(OBJ_DIR)/kernel
KERNEL_LINKERSCRIPT	:= $(KERNEL_SRC_DIR)/linker.lds
KERNEL_SRC_FILES	:= $(shell find $(KERNEL_SRC_DIR) -name "*.c")
KERNEL_OBJ_FILES	:= $(patsubst $(KERNEL_SRC_DIR)/%.c,$(KERNEL_BUILD_DIR)/%.o, $(KERNEL_SRC_FILES))

build: $(FINAL_IMG)


$(FINAL_IMG): $(KERNEL_ELF)
	@mkdir -p $(FS_DIR)/boot
	@mkdir -p $(FS_DIR)/boot/limine
	cp -v limine.conf limine-binary/limine-bios.sys limine-binary/limine-bios-cd.bin limine-binary/limine-uefi-cd.bin $(FS_DIR)/boot/limine/
	@mkdir -p $(FS_DIR)/EFI/BOOT
	cp -v limine-binary/BOOTX64.EFI $(FS_DIR)/EFI/BOOT/
	cp -v limine-binary/BOOTIA32.EFI $(FS_DIR)/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $(FS_DIR) -o $@
	./limine-binary/limine bios-install $@


$(KERNEL_ELF): $(KERNEL_OBJ_FILES)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T $(KERNEL_LINKERSCRIPT) -o $@ $^


$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: build
	qemu-system-x86_64 -cdrom $(FINAL_IMG) -serial stdio
