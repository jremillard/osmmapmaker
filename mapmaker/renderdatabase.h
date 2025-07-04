#pragma once

#include <QString>
#include <vector>
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

    /// Return all keys ordered by frequency of occurrence.
    std::vector<QString> allKeysByFrequency();

    /// Return keys for a specific data source ordered by frequency.
    std::vector<QString> keysByFrequency(const QString& dataSource);

    /// Return values for a given key/data source ordered by frequency.
    std::vector<QString> valuesByFrequency(const QString& dataSource, const QString& key);
};
