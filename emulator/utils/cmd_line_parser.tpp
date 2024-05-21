#ifndef CMDLINEPARSER_TPP
#define CMDLINEPARSER_TPP

#include "cmd_line_parser.hpp"

template<typename T>
T CmdLineParser::get(const std::string& key) const {
    auto it = args.find(key);
    if (it != args.end()) {
        return convert<T>(it->second);
    } else {
        // throw std::invalid_argument("Argument not found: " + key);
        return T{};
    }
}

template<typename T>
T CmdLineParser::convert(const std::string& value) const {
    std::istringstream iss(value);
    T result;
    iss >> std::noskipws >> result;
    if (iss.fail() || !iss.eof()) {
        throw std::invalid_argument("Failed to convert value: " + value);
    }
    return result;
}

#endif // CMDLINEPARSER_TPP
