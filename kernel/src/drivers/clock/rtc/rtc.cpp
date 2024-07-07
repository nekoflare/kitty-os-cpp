//
// Created by Piotr on 13.06.2024.
//

#include <firmware/acpi/acpi.hpp>
#include "rtc.hpp"

bool is_rtc_updating()
{
    outb(rtc_register_port, 0xa);
    auto status = (inb(rtc_data_port) & 0x80);
    return status;
}

uint8_t rtc_read(uint8_t rtc_register)
{
    outb(rtc_register_port, rtc_register);
    return inb(rtc_data_port);
}

uint8_t bcd_to_bin(uint8_t bcd)
{
    // Extract the high nibble (upper 4 bits) and multiply by 10
    // Extract the low nibble (lower 4 bits) and add to the result
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void rtc_get_time(ktime_t* time)
{
    while (!is_rtc_updating());

    auto seconds        = bcd_to_bin(rtc_read(0x00));
    auto minutes        = bcd_to_bin(rtc_read(0x02));
    auto hours          = bcd_to_bin(rtc_read(0x04));
    // auto week_day       = bcd_to_bin(rtc_read(0x06));
    auto day_of_month   = bcd_to_bin(rtc_read(0x07));
    auto month          = bcd_to_bin(rtc_read(0x08));
    auto year           = bcd_to_bin(rtc_read(0x09));
    uint32_t century = 20; // update this every century

    // Here, we should use ACPI to get century register.
    acpi_fadt* fadt = acpi_get_fadt();

    if (fadt != nullptr)
    {
        kstd::printf("Using FADT to get century register.\n");
        century = bcd_to_bin(rtc_read(fadt->Century));
    }

    time->second = seconds;
    time->minute = minutes;
    time->hour = hours;
    time->day = day_of_month;
    time->month = month;
    time->year = (century * 100) + year;
}