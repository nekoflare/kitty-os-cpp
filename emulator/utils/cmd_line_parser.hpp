#ifndef CMDLINEPARSER_H
#define CMDLINEPARSER_H

#include <unordered_map>
#include <string>
#include <stdexcept>
#include <sstream>

class CmdLineParser {
private:
    std::unordered_map<std::string, std::string> args;

public:
    CmdLineParser(int argc, char** argv);

    template<typename T>
    T get(const std::string& key) const;

private:
    template<typename T>
    T convert(const std::string& value) const;
};

#include "cmd_line_parser.tpp"

#endif // CMDLINEPARSER_H
