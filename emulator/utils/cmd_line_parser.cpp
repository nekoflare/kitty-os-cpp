#include "cmd_line_parser.hpp"

CmdLineParser::CmdLineParser(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        size_t eqPos = arg.find('=');
        if (eqPos != std::string::npos) {
            std::string key = arg.substr(0, eqPos);
            std::string value = arg.substr(eqPos + 1);
            args[key] = value;
        } else {
            throw std::invalid_argument("Invalid argument format: " + arg);
        }
    }
}
