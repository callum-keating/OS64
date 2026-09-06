.PHONY: build run clean


CC 			:= clang
CFLAGS 		:= --target=i686-elf -std=gnu23 -ffreestanding -O2 -Wall -Wextra

LD 			:= ld
LDFLAGS 	:= -m elf_i386

ASM			:= clang
ASMFLAGS 	:= --target=i686-elf


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
KERNEL_LINKERSCRIPT	:= $(KERNEL_SRC_DIR)/linker.ld
KERNEL_SRC_FILES	:= $(shell find $(KERNEL_SRC_DIR) -name "*.c")
KERNEL_OBJ_FILES	:= $(patsubst $(KERNEL_SRC_DIR)/%.c,$(KERNEL_BUILD_DIR)/%.o, $(KERNEL_SRC_FILES))

build: $(FINAL_IMG)


$(FINAL_IMG): $(KERNEL_ELF)
	@mkdir -p $(FS_DIR)/boot/grub
	cp grub.cfg $(FS_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(FS_DIR)


$(KERNEL_ELF): $(KERNEL_OBJ_FILES)
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) -c $(KERNEL_SRC_DIR)/boot.s -o $(KERNEL_BUILD_DIR)/boot.o
	$(LD) $(LDFLAGS) -T $(KERNEL_LINKERSCRIPT) -o $@ $^ $(KERNEL_BUILD_DIR)/boot.o
	@if grub-file --is-x86-multiboot $@; then \
		printf "\033[32msuccessfully compiled multiboot kernel\033[0m\n"; \
	else \
    	echo "error when compiling kernel"; \
    	exit 1; \
	fi



$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: build
	qemu-system-i386 -cdrom $(FINAL_IMG) -serial stdio
