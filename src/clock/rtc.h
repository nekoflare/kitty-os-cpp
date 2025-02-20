//
// Created by Neko on 25.01.2025.
//

#ifndef RTC_H
#define RTC_H
#include <cstdint>

struct system_date
{
    uint64_t year, month, day, hour, minute, second;
};

void initialize_rtc();
system_date get_system_date();

#endif // RTC_H
