#pragma once

#include "debug.hpp"
#include <functional>
#include <string>
#include <vector>

class TestGroup
{
  public:
    TestGroup(const std::string &group_name);

    void add_test(const std::string &test_name, std::function<bool()> test);

    std::pair<int, int> run();

  private:
    std::string group_name;
    std::vector<std::pair<std::string, std::function<bool()>>> tests;
    int successes = 0;
    int overall = 0;
};

class TestSuite
{
  public:
    TestSuite(); // Explicit declaration of the constructor

    // Add a test group
    TestGroup &group(const std::string &group_name);

    void run();

  private:
    std::vector<TestGroup> groups;
    int successes = 0;
    int overall = 0;
};