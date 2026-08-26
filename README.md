# OS64

## setup environment

To setup the environment you must have the following packages installed: mtools, dosfstools, qemu-system-x86_64, parted, ovmf, and git
To install the packages on debian run the following command
```bash
sudo apt install mtools dosfstools parted qemu-system-x86 git ovmf
```

Then run the following command to configure the environment
```bash
./setup.sh
```
After that you can run `make run` to run the OS and it hopefully will work

If that fails run `find /usr/share -name "*OVMF*"` to find an ovmf file and replace the `OVMF_LOCATION` variable in `Makefile` with an entry from that list
