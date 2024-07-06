//
// Created by Piotr on 13.06.2024.
//

#include <kstd/kstring.hpp>
#include <kernel/kbd.hpp>
#include <kernel/clock.hpp>
#include <functional>
#include "kt_command.hpp"

extern kt_command __kt_commands_array[];
extern kt_command __kt_commands_array_end[];

constexpr size_t cmdbuf_size = 8192;

void kt_parse_command_line(char* cmdbuf, kstd::vector<kstd::string>& output) {
    bool in_quotes = false;
    bool escaped = false;
    kstd::string tok;

    while (*cmdbuf != 0)
    {
        auto c  = *cmdbuf;
        cmdbuf++;

        if (c == '\"' && !escaped)
        {
            in_quotes = !in_quotes;
            continue;
        }

        if (!in_quotes && c == ' ' && !escaped)
        {
            if (!tok.empty())
            {
                output.push_back(tok);
                tok = "";
            }
            continue;
        }

        if (c == '\\' && !escaped)
        {
            escaped = true;
            continue;
        }

        // Normal character handling
        if (escaped)
        {
            // Handle escaped character
            if (c == 'n')
                tok += '\n';
            else if (c == 't')
                tok += '\t';
            else
                tok += c; // Add the character literally (including \ itself)

            escaped = false; // Reset escape flag after handling
        }
        else
        {
            tok += c; // Add the character to the current token
        }
    }

    // Add the last token if there's any left
    if (!tok.empty())
    {
        output.push_back(tok);
    }
}

void kt_do_comamnd()
{
    char cmdbuf[cmdbuf_size];

    kstd::printf("KernelTerminal ~ > ");
    kstd::memset(cmdbuf, 0, sizeof(cmdbuf));
    kbd_read(cmdbuf, true);
    kstd::putc('\n');

    kstd::vector<kstd::string> parsed_cmdline;
    kt_parse_command_line(cmdbuf, parsed_cmdline);

    if (parsed_cmdline.getSize() == 0)
    {
        return;
    }

    auto str = kstd::strdup(parsed_cmdline[0].c_str());
    auto command_name = kstd::string(str);

    if (!parsed_cmdline.empty()) {
        parsed_cmdline.erase(0);
    }

    size_t kt_commands = (__kt_commands_array_end - __kt_commands_array);

    for (size_t i = 0; kt_commands > i; i++)
    {
        auto kt_cmd = __kt_commands_array[i];
        
        if (kstd::strcmp(kt_cmd.command_name, str) == 0)
        {
            kt_cmd.command_function(command_name, parsed_cmdline);

            return;
        }
    }

    kstd::printf("Command \"%s\" not found.\n", str);
    delete str;
}

void kt_main()
{
    while (true)
    {
        kt_do_comamnd();
    }

    while (true);
}