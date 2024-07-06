//
// Created by Piotr on 17.06.2024.
//

#include <public/kdu/driver_ctrl.hpp>
#include "../kt_command.hpp"

void pwkd_command([[maybe_unused]] kstd::string& command_name, [[maybe_unused]] kstd::vector<kstd::string>& params)
{
    kstd::printf("Drivers: \n");
    driver_ctrl_print_ready_drivers();
}

kt_command_spec pwkd_command_desc = {
        .command_name = "Get-Working-Drivers",
        .command_function = &pwkd_command
};