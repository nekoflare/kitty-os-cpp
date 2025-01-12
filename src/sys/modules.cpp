#include <debug.hpp>
#include <limine.h>
#include <string.h>
#include <sys/modules.hpp>
#include <sys/symbols.hpp>

limine_module_request module_request = {.id = LIMINE_MODULE_REQUEST,
                                        .revision = 1,
                                        .response = nullptr,
                                        .internal_module_count = 0,
                                        .internal_modules = nullptr};

void print_modules()
{
    if (module_request.response == nullptr)
    {
        debug_printf("Failed to retrieve modules, response is null.\n");
        asm volatile("cli; hlt");
    }

    const auto &response = *module_request.response;

    for (uint64_t module_index = 0; response.module_count > module_index; module_index++)
    {
        const auto &module = *response.modules[module_index];

        debug_printf("Path: %s Command Line: %s size: %lu (B)\n", module.path, module.cmdline, module.size);
    }
}

kernel_module get_module(const char *path)
{
    if (module_request.response == nullptr)
        return {nullptr, 0};

    const auto &response = *module_request.response;

    for (uint64_t module_index = 0; response.module_count > module_index; module_index++)
    {
        const auto &module = *response.modules[module_index];

        if (strcmp(module.path, path) == 0)
            return {module.address, module.size};
    }

    return {nullptr, 0};
}

EXPORT_SYMBOL(get_module)