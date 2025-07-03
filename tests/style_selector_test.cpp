#include <catch2/catch_test_macros.hpp>
#include "stylelayer.h"

TEST_CASE("StyleSelector default constructor", "[StyleSelector]")
{
    StyleSelector sel;
    REQUIRE(sel.conditionCount() == 0);
    REQUIRE(sel.mapniKExpression() == "");
}

TEST_CASE("StyleSelector star expression and getters", "[StyleSelector]")
{
    StyleSelector sel("amenity");
    REQUIRE(sel.conditionCount() == 1);
    REQUIRE(sel.mapniKExpression() == "(true)");

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
    REQUIRE(sel.mapniKExpression() == "(true and ([name] = null))");

    std::vector<QString> vals = { "v1", "v2" };
    sel.setCondition(0, "key", vals);
    REQUIRE(sel.mapniKExpression() == "((([key]='v1') or ([key]='v2')) and ([name] = null))");

    sel.deleteCondition(1);
    REQUIRE(sel.mapniKExpression() == "((([key]='v1') or ([key]='v2')))");

    sel.clear();
    REQUIRE(sel.conditionCount() == 0);
    REQUIRE(sel.mapniKExpression() == "");
}
