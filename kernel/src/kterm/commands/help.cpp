//
// Created by Piotr on 13.06.2024.
//

#include <kstd/kstdio.hpp>
#include "../kt_command.hpp"

void help_cmd([[maybe_unused]] kstd::string& command_name, [[maybe_unused]] kstd::vector<kstd::string>& params)
{
    kstd::printf("Hello!\n");
}

kt_command_spec help_cmd_desc = {
        .command_name = "help",
        .command_function = &help_cmd
};