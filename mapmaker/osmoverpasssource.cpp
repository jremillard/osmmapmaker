#include "osmoverpasssource.h"
#include "osmiumimporter.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

QHash<QString, QByteArray> OsmOverpassSource::cache_;

OsmOverpassSource::OsmOverpassSource(QNetworkAccessManager* nam, QDomElement projectNode)
    : DataSource(projectNode)
    , nam_(nam)
{
    QDomElement qEl = projectNode.firstChildElement("overpassQuery");
    if (!qEl.isNull())
        query_ = qEl.text();
}

OsmOverpassSource::OsmOverpassSource(QNetworkAccessManager* nam)
    : nam_(nam)
{
}

OsmOverpassSource::~OsmOverpassSource() { }

void OsmOverpassSource::setQuery(const QString& query)
{
    query_ = query;
}

QString OsmOverpassSource::query() const
{
    return query_;
}

QByteArray OsmOverpassSource::download()
{
    if (cache_.contains(query_))
        return cache_[query_];

    if (!nam_ || nam_->networkAccessible() != QNetworkAccessManager::Accessible)
        throw std::runtime_error("Overpass network unavailable");

    QNetworkRequest req(QUrl("https://overpass-api.de/api/interpreter"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QEventLoop loop;
    QNetworkReply* reply = nam_->post(req, QByteArray("data=") + QUrl::toPercentEncoding(query_));
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QString msg = reply->errorString();
        reply->deleteLater();
        throw std::runtime_error(QString("Overpass error: %1").arg(msg).toStdString());
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    cache_.insert(query_, data);
    return data;
}

void OsmOverpassSource::importData(RenderDatabase& db)
{
    QByteArray data = download();
    OsmiumImporter::importBuffer(db, data, dataName());
}

void OsmOverpassSource::saveXML(QDomDocument& doc, QDomElement& toElement)
{
    toElement = doc.createElement("overpassSource");
    DataSource::saveXML(doc, toElement);
    QDomElement qEl = doc.createElement("overpassQuery");
    qEl.appendChild(doc.createTextNode(query_));
    toElement.appendChild(qEl);
}
