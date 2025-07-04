#include <catch2/catch_test_macros.hpp>
#include "textfield.h"
#include <set>
#include <map>

TEST_CASE("TextFieldProcessor RequiredKeys simple", "[TextFieldProcessor]")
{
    std::set<QString> keys;
    TextFieldProcessor::RequiredKeys("Hello [name]", &keys);
    REQUIRE(keys.size() == 1);
    REQUIRE(keys.count(QString("name")) == 1);
}

TEST_CASE("TextFieldProcessor RequiredKeys multiple and escaping", "[TextFieldProcessor]")
{
    std::set<QString> keys;
    TextFieldProcessor::RequiredKeys("Path [a,b] and \\[c]", &keys);
    REQUIRE(keys.size() == 2);
    REQUIRE(keys.count(QString("a")) == 1);
    REQUIRE(keys.count(QString("b")) == 1);
    REQUIRE(keys.count(QString("c")) == 0);
}

TEST_CASE("TextFieldProcessor Expand", "[TextFieldProcessor]")
{
    std::map<QString, QString> kv;
    kv["name"] = "OpenAI";
    REQUIRE(TextFieldProcessor::Expand("Hello [name]", kv) == "Hello OpenAI");
}

TEST_CASE("TextFieldProcessor Expand with fallback", "[TextFieldProcessor]")
{
    std::map<QString, QString> kv;
    kv["alt"] = "Buddy";
    REQUIRE(TextFieldProcessor::Expand("Hello [name, alt]", kv) == "Hello Buddy");
}

TEST_CASE("TextFieldProcessor Expand prefer first key", "[TextFieldProcessor]")
{
    std::map<QString, QString> kv;
    kv["name"] = "OpenAI";
    kv["alt"] = "Buddy";
    REQUIRE(TextFieldProcessor::Expand("Hello [name, alt]", kv) == "Hello OpenAI");
}

TEST_CASE("TextFieldProcessor Expand unknown key", "[TextFieldProcessor]")
{
    std::map<QString, QString> kv;
    REQUIRE(TextFieldProcessor::Expand("Hello [name]", kv) == "Hello ");
}

TEST_CASE("TextFieldProcessor Expand with escaping", "[TextFieldProcessor]")
{
    std::map<QString, QString> kv;
    kv["name"] = "OpenAI";
    REQUIRE(TextFieldProcessor::Expand("Show \\[name] and [name]", kv) == "Show \\[name] and OpenAI");
}

TEST_CASE("TextFieldProcessor Expand trims keys", "[TextFieldProcessor]")
{
    std::map<QString, QString> kv;
    kv["name"] = "OpenAI";
    kv["alt"] = "Buddy";
    REQUIRE(TextFieldProcessor::Expand("Hello [ name , alt ]", kv) == "Hello OpenAI");
}
