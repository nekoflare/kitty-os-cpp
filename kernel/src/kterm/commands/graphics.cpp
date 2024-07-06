//
// Created by Piotr on 17.06.2024.
//

#include <kstd/kstdio.hpp>
#include <public/kdu/driver_ctrl.hpp>
#include <public/kdu/apis/graphics.hpp>
#include "../kt_command.hpp"

void gfx_cmd(kstd::string& command_name, kstd::vector<kstd::string>& params)
{
    if (params.getSize() >= 1)
    {
        // Param 1
        if (kstd::strcmp(params[0].c_str(), "-EnumGfx") == 0)
        {
            kstd::printf("Available graphics adapters: \n");

            auto descriptors = driver_ctrl_get_descriptors();
            auto head = descriptors;

            while (head != nullptr)
            {
                if (head->driver->driver_designation == DT_GPU)
                {
                    kstd::printf("[UID %zu] %s\n", head->identifier, head->driver->driver_name);
                }
                head = head->next;
            }
        }
        else if (kstd::strcmp(params[0].c_str(), "-SetRes") == 0)
        {
            // Format: Gfx -SetRes [W] [H] [BPP] [GfxUID]

            if (params.getSize() != 5)
            {
                kstd::printf("Not enough parameters.\n");
                kstd::printf("Expected: Gfx -SetRes [Width] [Height] [Bpp] [GfxUID]\n");

                return;
            }

            auto width = kstd::strtoull(params[1].c_str(), 'd');
            auto height = kstd::strtoull(params[2].c_str(), 'd');
            auto bpp = kstd::strtoull(params[3].c_str(), 'd');
            auto gfxuid = kstd::strtoull(params[4].c_str(), 'd');

            GpuResolution new_res = {
                    .width = width,
                    .height = height,
                    .bpp = bpp
            };

            auto descriptors = driver_ctrl_get_descriptors();
            auto head = descriptors;

            while (head != nullptr)
            {
                if (head->identifier == gfxuid)
                {
                    head->driver->driver_ioctl(nullptr, GPU_SET_RESOLUTION, reinterpret_cast<const char*>(&new_res), nullptr);
                }
                head = head->next;
            }
        }
        else
        {
            kstd::printf("Unknown sub-command \"%s\" in command \"%s\".\n", params[0].c_str(), command_name.c_str());

            return;
        }
    }
}

kt_command_spec gfx_cmd_desc = {
        .command_name = "Gfx",
        .command_function = &gfx_cmd
};