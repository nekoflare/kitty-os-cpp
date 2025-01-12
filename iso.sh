git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1

mkdir iso_root/
mkdir iso_root/boot
mkdir iso_root/boot/limine/
mkdir iso_root/EFI/
mkdir iso_root/EFI/BOOT

cp -v limine/limine-bios.sys iso_root/boot/limine
cp -v limine/limine-bios-cd.bin iso_root/boot/limine
cp -v limine/limine-uefi-cd.bin iso_root/boot/limine
cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT
cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT
cp -v kernel.elf iso_root/kernel
cp -v limine.conf iso_root/boot/limine

cp -v kernel_modules/template/template.kmod iso_root/template.kmod

xorriso -as mkisofs \
            -b boot/limine/limine-bios-cd.bin \
            -no-emul-boot -boot-load-size 4 -boot-info-table \
            --efi-boot boot/limine/limine-uefi-cd.bin \
            -efi-boot-part --efi-boot-image --protective-msdos-label \
            -r -J -joliet-long -V "Kitty OS" \
            iso_root/ -o "image.iso"

./limine/limine bios-install image.iso

rm -rf iso_root/
