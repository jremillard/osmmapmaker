#pragma once

#include "osmdata.h"
#include <QNetworkAccessManager>
#include <QDomElement>

/// Downloads OSM data using the Overpass API and imports it.
class OsmDataOverpass : public OsmData {
public:
    explicit OsmDataOverpass(QNetworkAccessManager* nam, QDomElement projectNode);
    explicit OsmDataOverpass(QNetworkAccessManager* nam);
    ~OsmDataOverpass();

    void setQuery(const QString& query);
    QString query() const;

    void importData(SQLite::Database& db) override;

    void saveXML(QDomDocument& doc, QDomElement& toElement) override;

    static QHash<QString, QByteArray> cache_;

private:
    QByteArray download();

    QNetworkAccessManager* nam_;
    QString query_;
};
