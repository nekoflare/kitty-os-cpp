cmake_minimum_required(VERSION 3.10)

project(KittyOS VERSION 1.0 LANGUAGES C CXX ASM)

enable_language(ASM_NASM)

set(CMAKE_C_COMPILER /usr/bin/x86_64-linux-gnu-gcc-11)
set(CMAKE_CXX_COMPILER /usr/bin/x86_64-linux-gnu-g++-11)
set(CMAKE_ASM_NASM_COMPILER /usr/bin/nasm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS "-lgcc -Wl,--export-dynamic -I./src/ -Wall -O0 -fno-stack-protector -Wextra -ffreestanding -fno-stack-check -fno-lto -ffunction-sections -fdata-sections -m64 -msse -mno-mmx -mno-sse2 -fno-pic -mno-sse3 -mno-ssse3 -mno-avx -mno-avx2 -mno-avx512f -mno-red-zone -march=x86-64 -static -Isrc/ -Wformat-security -Wformat -g -mcmodel=kernel")
set(CMAKE_CXX_FLAGS "-lgcc -Wl,--export-dynamic -I./src/ -fpermissive -fno-rtti -Wall -O0 -fno-stack-protector -Wextra -ffreestanding -fno-stack-check -fno-lto -ffunction-sections -fdata-sections -fno-exceptions -fno-pic -m64 -msse -mno-mmx -mno-sse2 -mno-sse3 -mno-ssse3 -mno-avx -mno-avx2 -mno-avx512f -mno-red-zone -march=x86-64 -fno-threadsafe-statics -std=c++20 -Isrc/ -Wformat-security -Wformat -g -mcmodel=kernel")
set(CMAKE_EXE_LINKER_FLAGS "-lgcc -nostdlib -static -z text -T ../x86_64-linker.ld -Wl,--export-dynamic -z max-page-size=0x1000")
set(CMAKE_ASM_NASM_FLAGS "-f elf64")


file(GLOB_RECURSE SOURCES
        "src/*.c"
        "src/*.cpp"
        "src/*.asm"
        "src/*.h"
)

add_executable(KittyOS
        ${SOURCES}
        src/mem/physical.cpp
        src/mem/physical.h
        src/mem/virtual.cpp
        src/mem/virtual.h
        src/mem/heap.cpp
        src/mem/heap.h
        src/lib/klibc/runtime-blob.cpp
        src/lib/klibc/runtime-blob.h
        src/lib/npf/nanoprintf.cpp
        src/lib/npf/nanoprintf.h
        src/acpi/acpi.cpp
        src/acpi/acpi.h
        src/hal/apic/apic.cpp
        src/hal/apic/apic.h
        src/hal/ioapic/ioapic.cpp
        src/hal/ioapic/ioapic.h
        src/hal/irq/irq.cpp
        src/hal/irq/irq.h
        src/kernel.h
        src/smp/smp.cpp
        src/smp/smp.h
        src/hal/cpu/msr.h
        src/vfs/disk_interface.cpp
        src/vfs/disk_interface.h
        src/vfs/virt_fs.cpp
        src/vfs/virt_fs.h
        src/fs/filesystem.cpp
        src/fs/filesystem.h
        src/fs/fs/virt_kernel/vk_sys.cpp
        src/fs/fs/virt_kernel/vk_sys.h
        src/fs/fs/ext4/ext4.cpp
        src/fs/fs/ext4/ext4.h
        src/fs/mbr.cpp
        src/fs/mbr.h
        src/fs/gpt.cpp
        src/fs/gpt.h
        src/fs/utils.cpp
        src/fs/utils.h
        src/vfs/interfaces/vdi/vdi.cpp
        src/vfs/interfaces/vdi/vdi.h
        src/clock/clock.cpp
        src/clock/clock.h
        src/smp/sched.cpp
        src/smp/sched.h
        src/local_storage.cpp
        src/local_storage.h
        src/lib/klibc/spinlock.cpp
        src/lib/klibc/spinlock.h
        src/clock/rtc.cpp
        src/clock/rtc.h
        src/hal/pcie/pcie.cpp
        src/hal/pcie/pcie.h
        src/fb/framebuffer.cpp
        src/fb/framebuffer.h
        src/fb/image.cpp
        src/fb/image.h
        src/lib/stb/stb_image.h
        src/lib/stb/implementation.cpp
        src/lib/klibc/math.cpp
        src/lib/klibc/math.h
        src/main.cpp
        src/acpi/uacpi/tables.c
        src/acpi/uacpi/types.c
        src/acpi/uacpi/uacpi.c
        src/acpi/uacpi/utilities.c
        src/acpi/uacpi/interpreter.c
        src/acpi/uacpi/opcodes.c
        src/acpi/uacpi/namespace.c
        src/acpi/uacpi/stdlib.c
        src/acpi/uacpi/shareable.c
        src/acpi/uacpi/opregion.c
        src/acpi/uacpi/default_handlers.c
        src/acpi/uacpi/io.c
        src/acpi/uacpi/notify.c
        src/acpi/uacpi/sleep.c
        src/acpi/uacpi/registers.c
        src/acpi/uacpi/resources.c
        src/acpi/uacpi/event.c
        src/acpi/uacpi/mutex.c
        src/acpi/uacpi/osi.c
        src/acpi/uacpi_glue.cpp
        src/lib/klibc/cpu.cpp
        src/lib/klibc/cpu.h
        src/mem/malloc.c
)

include_directories(KittyOS PRIVATE src)
include_directories(KittyOS PRIVATE src/acpi/)
include_directories(KittyOS PRIVATE src/mem/liballoc/)
# target_include_directories(KittyOS PRIVATE src)

add_custom_target(PostBuild ALL
        COMMAND ${CMAKE_COMMAND} -E rename KittyOS kernel.elf
        COMMAND chmod +x ../iso.sh
        COMMAND ../iso.sh
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Post-build: Renaming KittyOS to kernel.elf and invoking iso.sh"
        DEPENDS KittyOS
)

add_custom_target(PerformAll ALL
    COMMAND qemu-system-x86_64.exe -cpu qemu64,+fsgsbase,+syscall -M smm=off -smp 6 -M q35 -m 2G -debugcon stdio -cdrom image.iso -drive if=pflash,unit=0,format=raw,file=./ovmf/ovmf-code-x86_64.fd,readonly=on -drive if=pflash,unit=1,format=raw,file=./ovmf/ovmf-vars-x86_64.fd
    DEPENDS PostBuild
)
