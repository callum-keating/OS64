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

function ekp() {
    nvim $kernel_dir/paging
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
