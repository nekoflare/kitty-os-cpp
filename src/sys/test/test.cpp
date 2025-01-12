#include <ansi.hpp>
#include <sys/test/test.hpp>

TestGroup::TestGroup(const std::string &group_name) : group_name(group_name)
{
}

void TestGroup::add_test(const std::string &test_name, std::function<bool()> test)
{
    tests.push_back({test_name, test});
}

std::pair<int, int> TestGroup::run()
{
    int success_count = 0;
    int total_count = 0;
    debug_printf("Running group: %s\n", group_name.c_str());

    for (const auto &[test_name, test] : tests)
    {
        ++total_count;
        bool result = test();
        if (result)
        {
            ++success_count;
            debug_printf("  Test '%s': " ANSI_GREEN "PASSED" ANSI_RESET "\n", test_name.c_str());
        }
        else
        {
            debug_printf("  Test '%s': " ANSI_RED "FAILED" ANSI_RESET "\n", test_name.c_str());
        }
    }

    debug_printf("Group '%s' Summary: %d/%d tests passed.\n", group_name.c_str(), success_count, total_count);
    this->successes += success_count;
    this->overall += total_count;

    return std::make_pair(this->successes, this->overall);
}

TestSuite::TestSuite()
{
}

TestGroup &TestSuite::group(const std::string &group_name)
{
    groups.emplace_back(group_name);
    return groups.back();
}

void TestSuite::run()
{
    for (auto &group : groups)
    {
        auto successes = group.run(); // Just run the group and its tests
        this->successes += successes.first;
        this->overall += successes.second;
    }

    debug_printf("Overall Summary: %d/%d tests passed.\n", successes, overall);
}