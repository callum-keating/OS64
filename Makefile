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
	-Isrc/kernel \
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
	dd if=/dev/zero of=$@ bs=1M count=64
	PATH=$PATH:/usr/sbin:/sbin sgdisk $@ -n 1:2048 -t 1:ef00 -m 1
	./limine-binary/limine bios-install $@
	mformat -i $@@@1M
	mmd -i $@@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i $@@@1M $^ ::/boot
	mcopy -i $@@@1M limine.conf limine-binary/limine-bios.sys ::/boot/limine
	mcopy -i $@@@1M limine-binary/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i $@@@1M limine-binary/BOOTIA32.EFI ::/EFI/BOOT


$(KERNEL_ELF): $(KERNEL_OBJ_FILES)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T $(KERNEL_LINKERSCRIPT) -o $@ $^


$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: build
	qemu-system-x86_64 -drive file=$(FINAL_IMG),format=raw -serial stdio -m 256M

run-uefi: build
	qemu-system-x86_64 -drive file=build/final.img,format=raw -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd -drive if=pflash,format=raw,file=./OVMF_VARS_4M.fd -serial stdio -m 256M
