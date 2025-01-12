# Architecture
arch := x86_64

# Compiler Paths
cxx := /usr/bin/x86_64-linux-gnu-g++-14
cc := /usr/bin/x86_64-linux-gnu-gcc-14
ld := /usr/bin/x86_64-linux-gnu-ld
asc := nasm
clang_format := clang-format
clang_tidy := clang-tidy

# Flags
cxx_flags := -fpermissive -fno-rtti -Wall -O0 -fno-stack-protector -Wextra -ffreestanding \
    -fno-stack-check -fno-lto -fno-PIC -ffunction-sections -fdata-sections \
    -I src/ -fno-exceptions -fPIE -DX86_64 -m64 -msse -mno-mmx \
    -mno-sse2 -mno-sse3 -mno-ssse3 -mno-avx -mno-avx2 -mno-avx512f \
    -mno-red-zone -march=x86-64 -fno-threadsafe-statics -std=c++20 \
    -Isrc/ -Wformat-security -Wformat -g -I src/acpi/ -mcmodel=large -falign-functions=64 -Isrc/sqlite
cc_flags := -Wall -O0 -fno-stack-protector -Wextra -ffreestanding \
    -fno-stack-check -fno-lto -fno-PIC -ffunction-sections -fdata-sections \
    -I src/ -DX86_64 -m64 -msse -mno-mmx -mno-sse2 -mno-sse3 -mno-ssse3 -I src/acpi/ \
    -mno-avx -mno-avx2 -mno-avx512f -mno-red-zone -march=x86-64 -static -Isrc/sqlite -Isrc/acpi/uacpi -Isrc/ -Wformat-security -Wformat -g -I src/flanterm -mcmodel=large -falign-functions=64
ld_flags := -z max-page-size=0x1000 -nostdlib -static -no-pie --no-dynamic-linker \
    -z text -T x86_64-linker.ld -m elf_x86_64 -gc-sections 
as_flags :=

# Directories
src_dir := src
obj_dir := obj

# Source files (include nested directories)
cpp_files := $(shell find $(src_dir) -type f -name '*.cpp')
c_files := $(shell find $(src_dir) -type f -name '*.c')
asm_files := $(shell find $(src_dir) -type f -name '*.asm')
hpp_files := $(shell find $(src_dir) -type f -name '*.hpp')

# Object files (preserve directory structure)
obj_files := $(patsubst $(src_dir)/%.cpp, $(obj_dir)/%.o, $(cpp_files)) \
             $(patsubst $(src_dir)/%.c, $(obj_dir)/%.o, $(c_files)) \
             $(patsubst $(src_dir)/%.asm, $(obj_dir)/%.o, $(asm_files))

# Target binary
target := kernel.elf

# Rules
.PHONY: all clean stats

all: $(target) stats

# Link the final binary
$(target): $(obj_files)
	@echo "Linking $(target)..."
	@time $(ld) $(ld_flags) -o $@ $^

# Compile C++ files
$(obj_dir)/%.o: $(src_dir)/%.cpp
	@mkdir -p $(dir $@)
	$(cxx) $(cxx_flags) -c $< -o $@

# Compile C files
$(obj_dir)/%.o: $(src_dir)/%.c
	@mkdir -p $(dir $@)
	$(cc) $(cc_flags) -c $< -o $@

# Assemble ASM files (preserve directory structure)
$(obj_dir)/%.o: $(src_dir)/%.asm
	@mkdir -p $(dir $@)
	$(asc) $(as_flags) -f elf64 $< -o $@

# Show ELF statistics
stats: $(target)
	@echo
	@echo "Section Sizes:"
	@size $(target)

# Clean up build artifacts
clean:
	rm -rf $(obj_dir) $(target)

# Format source files with clang-format
format: $(cpp_files) $(c_files)
	@echo "Running clang-format..."
	@$(clang_format) -i $(cpp_files)
	@$(clang_format) -i $(c_files)
	@$(clang_format) -i $(hpp_files)
