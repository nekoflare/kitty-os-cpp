//
// Created by Piotr on 13.06.2024.
//

#include <kstd/kstdio.hpp>
#include <drivers/clock/rtc/rtc.hpp>
#include "../kt_command.hpp"

void gettime_cmd([[maybe_unused]] kstd::string& command_name, [[maybe_unused]] kstd::vector<kstd::string>& params)
{
    ktime_t time {};
    rtc_get_time(&time);
    kstd::printf("(%zu:%zu:%zu) [%zu.%zu.%zu]\n", time.second, time.minute, time.hour, time.day, time.month, time.year);
}

kt_command_spec gettime_cmd_desc = {
        .command_name = "Get-Time",
        .command_function = &gettime_cmd
};