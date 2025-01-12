#!/usr/bin/bash
# Start QEMU with the necessary settings and enable GDB debugging
echo "Starting QEMU with debugging enabled..."
qemu-system-x86_64 \
    -M q35 \
    -m 4G \
    -smp 4 \
    -cdrom image.iso \
    -serial stdio \
    -d int \
    -s -S &  # Start QEMU with GDB server enabled on port 1234

# Give QEMU a moment to start up and wait for GDB connection
sleep 1

# Start GDB to connect to QEMU
echo "Starting GDB and connecting to QEMU..."
gdb -ex "target remote :1234" -ex "symbol-file kernel.elf" -ex "set disassembly-flavor intel" -ex "catch signal" -ex "catch throw"
