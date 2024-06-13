//
// Created by Piotr on 13.06.2024.
//

#ifndef KITTY_OS_CPP_KT_COMMAND_HPP
#define KITTY_OS_CPP_KT_COMMAND_HPP

#include <kstd/kvector.hpp>
#include <kstd/kstring.hpp>

typedef void (*kt_cmd_fn_pointer)(kstd::string&, kstd::vector<kstd::string>&);

typedef struct _KT_COMMAND
{
    const char* command_name;
    kt_cmd_fn_pointer command_function;
} kt_command;

#define kt_command_spec __attribute__((aligned(0x10))) __attribute__((section(".kt_commands"))) __attribute__((used)) static kt_command

#endif //KITTY_OS_CPP_KT_COMMAND_HPP
