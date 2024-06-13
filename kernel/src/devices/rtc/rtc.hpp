//
// Created by Piotr on 13.06.2024.
//

#ifndef KITTY_OS_CPP_RTC_HPP
#define KITTY_OS_CPP_RTC_HPP

#include <kernel/clock.hpp>
#include <kernel/hal/x64/io.hpp>

constexpr uint16_t rtc_io_port = 0x70;
constexpr uint16_t rtc_register_port = rtc_io_port;
constexpr uint16_t rtc_data_port = rtc_io_port + 1;

uint8_t rtc_read(uint8_t rtc_register);
void rtc_get_time(ktime_t* time);

#endif //KITTY_OS_CPP_RTC_HPP
