#include "osmdataoverpass.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

QHash<QString, QByteArray> OsmDataOverpass::cache_;

OsmDataOverpass::OsmDataOverpass(QNetworkAccessManager* nam, QDomElement projectNode)
    : OsmData(projectNode)
    , nam_(nam)
{
    QDomElement qEl = projectNode.firstChildElement("overpassQuery");
    if (!qEl.isNull())
        query_ = qEl.text();
}

OsmDataOverpass::OsmDataOverpass(QNetworkAccessManager* nam)
    : nam_(nam)
{
}

OsmDataOverpass::~OsmDataOverpass() { }

void OsmDataOverpass::setQuery(const QString& query)
{
    query_ = query;
}

QString OsmDataOverpass::query() const
{
    return query_;
}

QByteArray OsmDataOverpass::download()
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

void OsmDataOverpass::importData(RenderDatabase& db)
{
    QByteArray data = download();
    importBuffer(db, data);
}

void OsmDataOverpass::saveXML(QDomDocument& doc, QDomElement& toElement)
{
    toElement = doc.createElement("overpassSource");
    DataSource::saveXML(doc, toElement);
    QDomElement qEl = doc.createElement("overpassQuery");
    qEl.appendChild(doc.createTextNode(query_));
    toElement.appendChild(qEl);
}
