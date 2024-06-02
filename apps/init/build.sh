#!/usr/bin/env bash

CC="g++"
LD="ld"

CFLAGS="-I ../../libs/Kitty/include/ -I ../../libs/klibc++/include/ -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -m64 -march=x86-64 -mno-mmx -msse -msse2 -mno-red-zone -static -pedantic -std=c++20 -fno-rtti -fno-exceptions -fno-threadsafe-statics"
LDFLAGS="-m elf_x86_64 -nostdlib -static -no-pie --no-dynamic-linker -z text -z max-page-size=0x1000 -T ../linker_common.ld"

echo "Compiling initial program."

mkdir -p obj
mkdir -p bin

echo "Getting klibc++..."
cp ../../libs/klibc++/lib/klibc++.o obj/klibc++.o

$CC $CFLAGS src/main.cpp -c -o obj/main.cpp.o

# Linking process
$LD $LDFLAGS obj/klibc++.o obj/main.cpp.o -o bin/init.exe