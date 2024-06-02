#!/usr/bin/env bash

# Set compiler and linker
CC="g++"
LD="ld"

# Compilation and linker flags
CFLAGS="-I include/ -Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pie -m64 -march=x86-64 -mno-mmx -msse -msse2 -mno-red-zone -static -pedantic -std=c++20 -fno-rtti -fno-exceptions -fno-threadsafe-statics"
LDFLAGS="-m elf_x86_64 -nostdlib -static -no-pie --no-dynamic-linker -z text -z max-page-size=0x1000"

echo "Compiling klibc++ library."

# Create necessary directories
mkdir -p obj
mkdir -p lib

# Find all .cpp files in src and its subdirectories, compile them, and store the object files in obj directory
for file in $(find src -name "*.cpp"); do
    # Get the relative path and change .cpp to .o for the output file
    obj_file="obj/$(echo $file | sed 's|src/||; s|.cpp|.o|')"

    # Create the directory structure in obj if it doesn't exist
    mkdir -p $(dirname $obj_file)

    # Compile the source file into the corresponding object file
    $CC $CFLAGS -c $file -o $obj_file
done

# Find all .o files in obj directory and its subdirectories
obj_files=$(find obj -name "*.o")

# Link all object files into one big object file
$LD $LDFLAGS -r $obj_files -o lib/klibc++.o

echo "Compilation and linking completed."
