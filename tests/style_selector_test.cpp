#include <catch2/catch_test_macros.hpp>
#include "stylelayer.h"

TEST_CASE("StyleSelector default constructor", "[StyleSelector]")
{
    StyleSelector sel;
    REQUIRE(sel.conditionCount() == 0);
}

TEST_CASE("StyleSelector star expression and getters", "[StyleSelector]")
{
    StyleSelector sel("amenity");
    REQUIRE(sel.conditionCount() == 1);

    QString key;
    std::vector<QString> values;
    sel.condition(0, &key, &values);
    REQUIRE(key == "amenity");
    REQUIRE(values.size() == 1);
    REQUIRE(values[0] == "*");

    std::string keyStd;
    std::vector<std::string> valuesStd;
    sel.condition(0, &keyStd, &valuesStd);
    REQUIRE(keyStd == "amenity");
    REQUIRE(valuesStd.size() == 1);
    REQUIRE(valuesStd[0] == "*");
}

TEST_CASE("StyleSelector modify conditions", "[StyleSelector]")
{
    StyleSelector sel("initial");
    sel.insertCondition(1, "name", {});
    REQUIRE(sel.conditionCount() == 2);
    QString key;
    std::vector<QString> values;
    sel.condition(1, &key, &values);
    REQUIRE(key == "name");
    REQUIRE(values.empty());

    std::vector<QString> vals = { "v1", "v2" };
    sel.setCondition(0, "key", vals);
    sel.condition(0, &key, &values);
    REQUIRE(key == "key");
    REQUIRE(values == vals);

    sel.deleteCondition(1);
    REQUIRE(sel.conditionCount() == 1);
    sel.condition(0, &key, &values);
    REQUIRE(key == "key");
    REQUIRE(values == vals);

    sel.clear();
    REQUIRE(sel.conditionCount() == 0);
}
