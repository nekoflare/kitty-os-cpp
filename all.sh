set -e
make -j12

cd kernel_modules/template/
make clean
make -j12
cd ../../

./iso.sh
find src/ -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" -o -name "*.asm" -o -name "*.S" -o -name "*.inc" \) -exec cat {} + | wc -l
qemu-system-x86_64 -cpu qemu64,+fsgsbase,+syscall,+rdseed -no-shutdown -no-reboot -M smm=off -smp 6 -M q35 -m 128M -debugcon stdio -cdrom image.iso -drive if=pflash,unit=0,format=raw,file=ovmf/ovmf-code-x86_64.fd,readonly=on -drive if=pflash,unit=1,format=raw,file=ovmf/ovmf-vars-x86_64.fd
