#include "renderdatabase.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <QFile>
#include <fstream>
#include <sstream>
#include <stdexcept>

void execSqlFile(SQLite::Database& db, const std::string& path)
{
    std::ifstream in(path);
    if (!in.good())
        throw std::runtime_error("Cannot open SQL file: " + path);
    std::stringstream ss;
    ss << in.rdbuf();
    db.exec(ss.str());
}

static void execSqlResource(SQLite::Database& db, const QString& resource)
{
    QFile file(resource);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Cannot open resource: " + resource.toStdString());
    QByteArray sql = file.readAll();
    db.exec(sql.constData());
}

RenderDatabase::RenderDatabase()
    : db_(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
    upgrade();
}

RenderDatabase::RenderDatabase(const QString& filePath)
    : db_(filePath.toStdString(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
    upgrade();
}

SQLite::Database& RenderDatabase::db()
{
    return db_;
}

void RenderDatabase::upgrade()
{
    try {
        db_.exec("SELECT 1 FROM version LIMIT 1");
    } catch (...) {
        execSqlResource(db_, ":/resources/render-0.sql");
    }

    int currentVersion = 0;
    try {
        SQLite::Statement st(db_, "SELECT version FROM version");
        if (st.executeStep())
            currentVersion = st.getColumn(0).getInt();
    } catch (...) {
    }

    for (int rev = currentVersion + 1;; ++rev) {
        QString res = QStringLiteral(":/resources/render-%1.sql").arg(rev);
        QFile file(res);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            break;
        QByteArray sql = file.readAll();
        db_.exec(sql.constData());
        SQLite::Statement q(db_, "UPDATE version SET version = ?");
        SQLite::bind(q, rev);
        q.exec();
    }
}
