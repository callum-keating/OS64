kernel_dir=src/kernel

function ek() {
    nvim $kernel_dir
}

function ekm() {
    nvim $kernel_dir/main.c
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
