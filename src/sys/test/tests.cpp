#include <algorithm> // For std::transform
#include <cmath>
#include <cmath>      // For std::fabs (used in float comparison)
#include <functional> // For std::greater (used with std::priority_queue)
#include <map>        // For std::map
#include <queue>      // For std::queue and std::priority_queue
#include <set>        // For std::set
#include <stack>      // For std::stack
#include <string>     // For std::string
#include <sys/test/test.hpp>
#include <sys/test/tests.hpp>
#include <unordered_map> // For std::unordered_map
#include <vector>        // For std::vector

void add_string_tests(TestSuite &suite)
{
    // Create a group for String tests
    auto &string_tests = suite.group("String tests");

    // Test 1: Test string construction
    string_tests.add_test("Test string construction", [&]() -> bool {
        std::string str1;
        std::string str2("Hello");
        std::string str3(str2);
        std::string str4 = str3;
        return str1.empty() && str2 == "Hello" && str3 == "Hello" && str4 == "Hello";
    });

    // Test 2: Test string assignment
    string_tests.add_test("Test string assignment", [&]() -> bool {
        std::string str1;
        str1 = "World";
        return str1 == "World";
    });

    // Test 3: Test string concatenation
    string_tests.add_test("Test string concatenation", [&]() -> bool {
        std::string str1 = "Hello";
        std::string str2 = " World";
        std::string result = str1 + str2;
        return result == "Hello World";
    });

    // Test 4: Test string length
    string_tests.add_test("Test string length", [&]() -> bool {
        std::string str = "Hello";
        return str.length() == 5 && str.size() == 5;
    });

    // Test 5: Test string comparison (==, !=, <, >)
    string_tests.add_test("Test string comparison", [&]() -> bool {
        std::string str1 = "apple";
        std::string str2 = "banana";
        std::string str3 = "apple";
        return (str1 == str3) && (str1 != str2) && (str1 < str2) && (str2 > str1);
    });

    // Test 6: Test string substring
    string_tests.add_test("Test string substring", [&]() -> bool {
        std::string str = "Hello World";
        return str.substr(0, 5) == "Hello" && str.substr(6, 5) == "World";
    });

    // Test 7: Test string find
    string_tests.add_test("Test string find", [&]() -> bool {
        std::string str = "Hello World";
        return (str.find("World") == 6) && (str.find("world") == std::string::npos);
    });

    // Test 8: Test string character access
    string_tests.add_test("Test string character access", [&]() -> bool {
        std::string str = "Hello";
        return str[0] == 'H' && str.at(1) == 'e' && str.back() == 'o';
    });

    // Test 9: Test string append
    string_tests.add_test("Test string append", [&]() -> bool {
        std::string str = "Hello";
        str.append(" World");
        return str == "Hello World";
    });

    // Test 10: Test string insert
    string_tests.add_test("Test string insert", [&]() -> bool {
        std::string str = "Hell World";
        str.insert(4, "o");
        return str == "Hello World";
    });

    // Test 11: Test string erase
    string_tests.add_test("Test string erase", [&]() -> bool {
        std::string str = "Hello World";
        str.erase(5, 6); // Erase " World"
        return str == "Hello";
    });

    // Test 12: Test string replace
    string_tests.add_test("Test string replace", [&]() -> bool {
        std::string str = "Hello World";
        str.replace(6, 5, "C++");
        return str == "Hello C++";
    });

    // Test 13: Test string to upper case (using transform)
    string_tests.add_test("Test string to upper case", [&]() -> bool {
        std::string str = "Hello World";
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str == "HELLO WORLD";
    });

    // Test 14: Test string to lower case (using transform)
    string_tests.add_test("Test string to lower case", [&]() -> bool {
        std::string str = "Hello World";
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str == "hello world";
    });

    // Test 15: Test string conversion to integer (std::stoi)
    string_tests.add_test("Test string to integer conversion", [&]() -> bool {
        std::string str = "123";
        int num = std::stoi(str);
        return num == 123;
    });

    // Test 16: Test string conversion to float (std::stof)
    string_tests.add_test("Test string to float conversion", [&]() -> bool {
        std::string str = "3.14";
        float num = std::stof(str);
        const float tolerance = 1e-5f;
        return std::fabs(num - 3.14f) <= tolerance;
    });

    // Test 17: Test string with empty string check
    string_tests.add_test("Test string empty check", [&]() -> bool {
        std::string str1 = "";
        std::string str2 = "Non-empty";
        return str1.empty() && !str2.empty();
    });
}

void add_vector_tests(TestSuite &suite)
{
    // Create a group for Vector tests
    auto &vector_tests = suite.group("Vector tests");

    // Test 1: Test vector construction
    vector_tests.add_test("Test vector construction", [&]() -> bool {
        std::vector<int> vec1;
        std::vector<int> vec2(5, 42);
        std::vector<int> vec3 = {1, 2, 3};
        return vec1.empty() && vec2.size() == 5 && vec2[0] == 42 && vec3 == std::vector<int>{1, 2, 3};
    });

    // Test 2: Test vector push_back
    vector_tests.add_test("Test vector push_back", [&]() -> bool {
        std::vector<int> vec;
        vec.push_back(1);
        vec.push_back(2);
        return vec == std::vector<int>{1, 2};
    });

    // Test 3: Test vector size and capacity
    vector_tests.add_test("Test vector size and capacity", [&]() -> bool {
        std::vector<int> vec;
        vec.reserve(10);
        vec.push_back(1);
        return vec.size() == 1 && vec.capacity() >= 10;
    });

    // Test 4: Test vector element access
    vector_tests.add_test("Test vector element access", [&]() -> bool {
        std::vector<int> vec = {1, 2, 3};
        return vec[0] == 1 && vec.at(1) == 2 && vec.back() == 3;
    });

    // Test 5: Test vector insert and erase
    vector_tests.add_test("Test vector insert and erase", [&]() -> bool {
        std::vector<int> vec = {1, 3};
        vec.insert(vec.begin() + 1, 2);
        vec.erase(vec.begin());
        return vec == std::vector<int>{2, 3};
    });

    // Test 6: Test vector clear
    vector_tests.add_test("Test vector clear", [&]() -> bool {
        std::vector<int> vec = {1, 2, 3};
        vec.clear();
        return vec.empty();
    });
}

void add_unordered_map_tests(TestSuite &suite)
{
    // Create a group for Unordered Map tests
    auto &map_tests = suite.group("Unordered Map tests");

    // Test 1: Test unordered_map construction
    map_tests.add_test("Test unordered_map construction", [&]() -> bool {
        std::unordered_map<std::string, int> map = {{"a", 1}, {"b", 2}};
        return map.size() == 2 && map["a"] == 1 && map["b"] == 2;
    });

    // Test 2: Test unordered_map insertion
    map_tests.add_test("Test unordered_map insertion", [&]() -> bool {
        std::unordered_map<std::string, int> map;
        map["a"] = 1;
        map.insert({"b", 2});
        return map.size() == 2 && map["a"] == 1 && map["b"] == 2;
    });

    // Test 3: Test unordered_map find
    map_tests.add_test("Test unordered_map find", [&]() -> bool {
        std::unordered_map<std::string, int> map = {{"a", 1}, {"b", 2}};
        auto it = map.find("a");
        return it != map.end() && it->second == 1 && map.find("c") == map.end();
    });

    // Test 4: Test unordered_map erase
    map_tests.add_test("Test unordered_map erase", [&]() -> bool {
        std::unordered_map<std::string, int> map = {{"a", 1}, {"b", 2}};
        map.erase("a");
        return map.size() == 1 && map.find("a") == map.end();
    });

    // Test 5: Test unordered_map key iteration
    map_tests.add_test("Test unordered_map iteration", [&]() -> bool {
        std::unordered_map<std::string, int> map = {{"a", 1}, {"b", 2}};
        int sum = 0;
        for (const auto &pair : map)
            sum += pair.second;
        return sum == 3;
    });
}

void add_stack_tests(TestSuite &suite)
{
    // Create a group for Stack tests
    auto &stack_tests = suite.group("Stack tests");

    // Test 1: Test stack push and top
    stack_tests.add_test("Test stack push and top", [&]() -> bool {
        std::stack<int> stack;
        stack.push(1);
        stack.push(2);
        return stack.top() == 2;
    });

    // Test 2: Test stack pop
    stack_tests.add_test("Test stack pop", [&]() -> bool {
        std::stack<int> stack;
        stack.push(1);
        stack.push(2);
        stack.pop();
        return stack.top() == 1;
    });

    // Test 3: Test stack empty
    stack_tests.add_test("Test stack empty", [&]() -> bool {
        std::stack<int> stack;
        return stack.empty();
    });

    // Test 4: Test stack size
    stack_tests.add_test("Test stack size", [&]() -> bool {
        std::stack<int> stack;
        stack.push(1);
        stack.push(2);
        return stack.size() == 2;
    });
}

void add_queue_tests(TestSuite &suite)
{
    // Create a group for Queue tests
    auto &queue_tests = suite.group("Queue tests");

    // Test 1: Test queue push and front
    queue_tests.add_test("Test queue push and front", [&]() -> bool {
        std::queue<int> queue;
        queue.push(1);
        queue.push(2);
        return queue.front() == 1;
    });

    // Test 2: Test queue pop
    queue_tests.add_test("Test queue pop", [&]() -> bool {
        std::queue<int> queue;
        queue.push(1);
        queue.push(2);
        queue.pop();
        return queue.front() == 2;
    });

    // Test 3: Test queue empty
    queue_tests.add_test("Test queue empty", [&]() -> bool {
        std::queue<int> queue;
        return queue.empty();
    });

    // Test 4: Test queue size
    queue_tests.add_test("Test queue size", [&]() -> bool {
        std::queue<int> queue;
        queue.push(1);
        queue.push(2);
        return queue.size() == 2;
    });
}

void do_tests()
{
    TestSuite suite;

    // Add string tests to the suite
    add_string_tests(suite);
    add_string_tests(suite);
    add_vector_tests(suite);
    add_unordered_map_tests(suite);
    add_stack_tests(suite);
    add_queue_tests(suite);

    // Run the test suite
    suite.run();
}