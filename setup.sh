# Download the latest Limine binary release.
curl -L https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz | gunzip | tar -xf -

# Build "limine" utility.
make -C limine-binary
