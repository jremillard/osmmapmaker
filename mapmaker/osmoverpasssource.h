#pragma once

#include "datasource.h"
#include "osmiumimporter.h"
#include <QNetworkAccessManager>
#include <QDomElement>

/// Downloads OSM data using the Overpass API and imports it.
class OsmOverpassSource : public DataSource {
public:
    explicit OsmOverpassSource(QNetworkAccessManager* nam, QDomElement projectNode);
    explicit OsmOverpassSource(QNetworkAccessManager* nam);
    ~OsmOverpassSource();

    void setQuery(const QString& query);
    QString query() const;

    void importData(RenderDatabase& db) override;

    void saveXML(QDomDocument& doc, QDomElement& toElement) override;

    static QHash<QString, QByteArray> cache_;

private:
    QByteArray download();

    QNetworkAccessManager* nam_;
    QString query_;
};
