#pragma once

#include <QString>
#include <string>
#include <SQLiteCpp/SQLiteCpp.h>

namespace SQLite {
class Database;
}

// Execute SQL statements from a file path.
void execSqlFile(SQLite::Database& db, const std::string& path);

/// Helper that manages the render database schema upgrades.
class RenderDatabase {
public:
    /// Create an in-memory database.
    RenderDatabase();
    /// Create or open a database at the given path.
    explicit RenderDatabase(const QString& filePath);

    SQLite::Database& db();

private:
    void upgrade();

    SQLite::Database db_;
};
