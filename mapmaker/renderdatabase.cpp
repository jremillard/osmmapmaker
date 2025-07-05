#include "renderdatabase.h"

#include <QFile>
#include <SQLiteCpp/VariadicBind.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

void execSqlFile(RenderDatabase& db, const std::string& path)
{
    std::ifstream in(path);
    if (!in.good())
        throw std::runtime_error("Cannot open SQL file: " + path);
    std::stringstream ss;
    ss << in.rdbuf();
    db.exec(ss.str());
}

static void execSqlResource(RenderDatabase& db, const QString& resource)
{
    QFile file(resource);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Cannot open resource: " + resource.toStdString());
    QByteArray sql = file.readAll();
    db.exec(sql.constData());
}

RenderDatabase::RenderDatabase()
    : SQLite::Database(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
    Q_INIT_RESOURCE(mapmaker_resources);
    upgrade();
    exec("PRAGMA cache_size = -256000");
    exec("PRAGMA default_cache_size = 256000");
}

RenderDatabase::RenderDatabase(const QString& filePath)
    : SQLite::Database(filePath.toStdString(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
    Q_INIT_RESOURCE(mapmaker_resources);
    upgrade();
    exec("PRAGMA cache_size = -256000");
    exec("PRAGMA default_cache_size = 256000");
}

void RenderDatabase::upgrade()
{
    try {
        exec("SELECT 1 FROM version LIMIT 1");
    } catch (...) {
        execSqlResource(*this, ":/resources/render-0.sql");
    }

    int currentVersion = schemaVersion();
    int latest = latestVersion();
    if (currentVersion > latest)
        throw std::runtime_error("Render database schema too new");

    for (int rev = currentVersion + 1; rev <= latest; ++rev) {
        QString res = QStringLiteral(":/resources/render-%1.sql").arg(rev);
        execSqlResource(*this, res);
        SQLite::Statement q(*this, "UPDATE version SET version = ?");
        SQLite::bind(q, rev);
        q.exec();
    }
}

int RenderDatabase::schemaVersion()
{
    try {
        SQLite::Statement st(*this, "SELECT version FROM version");
        if (st.executeStep())
            return st.getColumn(0).getInt();
    } catch (...) {
    }
    return -1;
}

int RenderDatabase::latestVersion()
{
    int rev = -1;
    for (int i = 0;; ++i) {
        QString res = QStringLiteral(":/resources/render-%1.sql").arg(i);
        QFile file(res);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            break;
        rev = i;
    }
    return rev;
}

RenderDatabase::SchemaStatus RenderDatabase::schemaStatus()
{
    int current = schemaVersion();
    int latest = latestVersion();
    if (current == latest)
        return SchemaStatus::Compatible;
    if (current > latest)
        return SchemaStatus::Incompatible;
    // check upgrade scripts
    for (int rev = current + 1; rev <= latest; ++rev) {
        QString res = QStringLiteral(":/resources/render-%1.sql").arg(rev);
        QFile file(res);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return SchemaStatus::Incompatible;
    }
    return SchemaStatus::Upgradeable;
}

std::vector<QString> RenderDatabase::allKeysByFrequency()
{
    std::vector<QString> keys;
    SQLite::Statement query(*this, "select key, count(key) as freq from entityKV group by key order by freq desc");
    while (query.executeStep()) {
        keys.push_back(query.getColumn(0).getText());
    }
    return keys;
}

std::vector<QString> RenderDatabase::keysByFrequency(const QString& dataSource)
{
    std::vector<QString> keys;
    SQLite::Statement query(*this, "select key, count(key) as freq from entityKV, entity where entity.source = ? and entity.id = entityKV.id group by key order by freq desc");
    query.bind(1, dataSource.toStdString());
    while (query.executeStep()) {
        keys.push_back(query.getColumn(0).getText());
    }
    return keys;
}

std::vector<QString> RenderDatabase::valuesByFrequency(const QString& dataSource, const QString& key)
{
    std::vector<QString> values;
    SQLite::Statement query(*this, "select value, count(value) as freq from entityKV, entity where entity.source = ? and entity.id = entityKV.id and entityKV.key = ? group by value order by freq desc");
    query.bind(1, dataSource.toStdString());
    query.bind(2, key.toStdString());
    while (query.executeStep()) {
        values.push_back(query.getColumn(0).getText());
    }
    return values;
}

void RenderDatabase::cleanDataSource(const QString& dataSource)
{
    // clean out the entity table, entityKV records are removed via trigger
    SQLite::Statement removeData(*this, "DELETE FROM entity WHERE source = ?");
    removeData.bind(1, dataSource.toStdString());
    removeData.exec();

    // clean out the spatial index - cannot use trigger on virtual table
    SQLite::Statement removeIndex(
        *this,
        "DELETE FROM entitySpatialIndex WHERE NOT EXISTS (SELECT * FROM entitySpatialIndex,entity WHERE entitySpatialIndex.pkid = entity.id)");
    removeIndex.exec();
}
