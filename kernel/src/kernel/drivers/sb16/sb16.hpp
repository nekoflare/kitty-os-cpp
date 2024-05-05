//
// Created by Piotr on 05.05.2024.
//

#ifndef KITTY_OS_CPP_SB16_HPP
#define KITTY_OS_CPP_SB16_HPP

#include <kernel/hal/x64/io.hpp>

#define DSP_RESET 0x226
#define DSP_READ 0x22A
#define DSP_WRITE 0x22C
#define DSP_BUFFER 0x22E
#define DSP_INTERRUPT 0x22F

bool is_sb16_available();
void dsp_write(kstd::uint8_t value);
kstd::uint8_t dsp_read();
void test(int frequency);

#endif //KITTY_OS_CPP_SB16_HPP
