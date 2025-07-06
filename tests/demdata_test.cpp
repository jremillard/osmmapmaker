#include <catch2/catch_test_macros.hpp>
#include "demdata.h"
#include "renderdatabase.h"

TEST_CASE("DemData imports contour lines", "[DemData]")
{
    RenderDatabase db;
    DemData dem;
    dem.setFileName(QStringLiteral(SOURCE_DIR "/tests/dem/simple.asc"));
    dem.setInterval(1.0);
    dem.importData(db);

    SQLite::Statement count(db, "SELECT COUNT(*) FROM entity");
    REQUIRE(count.executeStep());
    REQUIRE(count.getColumn(0).getInt() > 0);

    SQLite::Statement kv(db, "SELECT value FROM entityKV ORDER BY id");
    bool found = false;
    while (kv.executeStep()) {
        QString val = kv.getColumn(0).getText();
        if (val == "1")
            found = true;
    }
    REQUIRE(found);
}
