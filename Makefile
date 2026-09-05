.PHONY: all build run clean

BUILD := build
ISO_DIR := $(BUILD)/iso
GRUB_DIR := $(ISO_DIR)/boot/grub

KERNEL := $(BUILD)/kernel.elf
FINAL_IMG := $(BUILD)/kernel.iso

SRC_DIR := src
OBJ_DIR := $(BUILD)/objs
KERNEL_DIR := $(SRC_DIR)/kernel

C_SRCS := $(shell find $(KERNEL_DIR) -name "*.c")
ASM_SRCS := $(shell find $(KERNEL_DIR) -name "*.S")

C_OBJS := $(patsubst $(KERNEL_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SRCS))
ASM_OBJS := $(patsubst $(KERNEL_DIR)/%.S,$(OBJ_DIR)/%.o,$(ASM_SRCS))

KERNEL_OBJS := $(C_OBJS) $(ASM_OBJS)

CC := clang
LD := ld

CFLAGS := \
	-Wall \
	-Wextra \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-mno-red-zone \
	-m64

ASFLAGS := \
	-m64

LDFLAGS := \
	-T linker.ld \
	-m elf_x86_64

all: build

build: $(FINAL_IMG)

$(FINAL_IMG): $(KERNEL)
	@mkdir -p $(GRUB_DIR)
	cp $(KERNEL) $(ISO_DIR)/boot/kernel.elf
	cp grub.cfg $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL): $(KERNEL_OBJS)
	@mkdir -p $(BUILD)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(KERNEL_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

run: build
	qemu-system-x86_64 -cdrom $(FINAL_IMG)

clean:
	rm -rf $(BUILD)

