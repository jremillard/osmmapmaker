#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("Hello world", "[hello]") {
    std::string message = "hello world";
    REQUIRE(message == "hello world");
}
