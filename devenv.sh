kernel_dir=src/kernel

function ek() {
    nvim $kernel_dir
}

function ekm() {
    nvim $kernel_dir/main.c
}

function ekd() {
    nvim $kernel_dir/drivers
}

function ekl() {
    nvim $kernel_dir/limine
}

function ekmem() {
    nvim $kernel_dir/memory
}

function ekmemp() {
    nvim $kernel_dir/memory/pmm.c
}

function em() {
    nvim Makefile
}

function r() {
    make run
}

function b() {
    make build
}

function c() {
    make clean
}
