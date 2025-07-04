#pragma once

#include <QString>
#include <string>
#include <SQLiteCpp/SQLiteCpp.h>

// Execute SQL statements from a file path.
class RenderDatabase;
void execSqlFile(RenderDatabase& db, const std::string& path);

/// Helper that manages the render database schema upgrades.
class RenderDatabase : public SQLite::Database {
public:
    /// Create an in-memory database.
    RenderDatabase();
    /// Create or open a database at the given path.
    explicit RenderDatabase(const QString& filePath);

    enum class SchemaStatus { Compatible,
        Upgradeable,
        Incompatible };

    int schemaVersion();
    int latestVersion();
    SchemaStatus schemaStatus();
    void upgrade();
};
