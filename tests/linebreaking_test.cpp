#include <catch2/catch_test_macros.hpp>
#include "linebreaking.h"
#include <vector>

TEST_CASE("breakLines handles empty input", "[linebreaking]")
{
    std::vector<int> words;
    std::vector<size_t> breaks;
    breakLines(words, 10, &breaks);
    REQUIRE(breaks.empty());
}

TEST_CASE("breakLines no breaks when line within width", "[linebreaking]")
{
    std::vector<int> words = { 1, 2, 3 };
    std::vector<size_t> breaks;
    breakLines(words, 10, &breaks);
    REQUIRE(breaks.empty());
}

TEST_CASE("breakLines single break", "[linebreaking]")
{
    std::vector<int> words = { 4, 5, 6 };
    std::vector<size_t> breaks;
    breakLines(words, 10, &breaks);
    REQUIRE(breaks == std::vector<size_t> { 2 });
}

TEST_CASE("breakLines multiple breaks", "[linebreaking]")
{
    std::vector<int> words = { 3, 5, 6, 4, 5 };
    std::vector<size_t> breaks;
    breakLines(words, 10, &breaks);
    REQUIRE(breaks == std::vector<size_t> { 2, 4 });
}
