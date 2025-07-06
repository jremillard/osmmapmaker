#include "mapcanvas.h"
#include "maputils.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QNetworkReply>

namespace {
constexpr int kTileSize = 256;
}

MapCanvas::MapCanvas(QWidget* parent)
    : QWidget(parent)
    , tileCache_(200)
{
    setMinimumSize(300, 200);
}

void MapCanvas::setCenter(double lat, double lon)
{
    centerLat_ = lat;
    centerLon_ = lon;
    update();
}

void MapCanvas::setZoom(int z)
{
    zoom_ = std::clamp(z, 0, 19);
    update();
}

QPointF MapCanvas::lonLatToPixel(double lat, double lon) const
{
    TileXY tile = lonLatToTile(lon, lat, zoom_);
    return QPointF(tile.x * kTileSize, tile.y * kTileSize);
}

void MapCanvas::pixelToLonLat(const QPointF& p, double& lat, double& lon) const
{
    TileXY t { p.x() / kTileSize, p.y() / kTileSize };
    tileToLonLat(t, zoom_, lon, lat);
}

void MapCanvas::requestTile(int x, int y) const
{
    QString key = QString("%1/%2/%3").arg(zoom_).arg(x).arg(y);
    if (tileCache_.contains(key) || pending_.contains(key))
        return;
    pending_.insert(key);
    QNetworkRequest req(QUrl(QString("https://tile.openstreetmap.org/%1/%2/%3.png").arg(zoom_).arg(x).arg(y)));
    QNetworkReply* reply = nam_.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, key]() {
        pending_.remove(key);
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap* px = new QPixmap;
            px->loadFromData(reply->readAll());
            tileCache_.insert(key, px);
        }
        reply->deleteLater();
        const_cast<MapCanvas*>(this)->update();
    });
}

void MapCanvas::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QPointF centerPx = lonLatToPixel(centerLat_, centerLon_);
    QPointF topLeft = centerPx - QPointF(width() / 2.0, height() / 2.0);

    int x0 = int(std::floor(topLeft.x() / kTileSize));
    int y0 = int(std::floor(topLeft.y() / kTileSize));
    int x1 = int(std::floor((topLeft.x() + width()) / kTileSize));
    int y1 = int(std::floor((topLeft.y() + height()) / kTileSize));

    for (int x = x0; x <= x1; ++x) {
        for (int y = y0; y <= y1; ++y) {
            int tx = (x % int(std::pow(2, zoom_)) + int(std::pow(2, zoom_))) % int(std::pow(2, zoom_));
            int ty = (y % int(std::pow(2, zoom_)) + int(std::pow(2, zoom_))) % int(std::pow(2, zoom_));
            QString key = QString("%1/%2/%3").arg(zoom_).arg(tx).arg(ty);
            QPixmap* pix = tileCache_.object(key);
            if (!pix)
                requestTile(tx, ty);
            if (pix)
                p.drawPixmap(x * kTileSize - topLeft.x(), y * kTileSize - topLeft.y(), *pix);
        }
    }

    if (selecting_ || (currentBox_.maxLat > currentBox_.minLat && currentBox_.maxLon > currentBox_.minLon)) {
        QRect rect(selectStart_, selectEnd_);
        p.setPen(Qt::red);
        QColor fill = Qt::red;
        fill.setAlpha(50);
        p.fillRect(rect.normalized(), fill);
        p.drawRect(rect.normalized());
    }
}

void MapCanvas::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ShiftModifier)) {
        selecting_ = true;
        selectStart_ = e->pos();
        selectEnd_ = e->pos();
        update();
    } else if (e->button() == Qt::LeftButton) {
        panning_ = true;
        panStart_ = e->pos();
        panLatStart_ = centerLat_;
        panLonStart_ = centerLon_;
    }
}

void MapCanvas::mouseMoveEvent(QMouseEvent* e)
{
    if (panning_) {
        QPointF delta = e->pos() - panStart_;
        QPointF centerPx = lonLatToPixel(panLatStart_, panLonStart_);
        centerPx -= delta;
        pixelToLonLat(centerPx, centerLat_, centerLon_);
        update();
    } else if (selecting_) {
        selectEnd_ = e->pos();
        update();
    }
}

void MapCanvas::mouseReleaseEvent(QMouseEvent* e)
{
    if (panning_ && e->button() == Qt::LeftButton) {
        panning_ = false;
    } else if (selecting_ && e->button() == Qt::LeftButton) {
        selecting_ = false;
        selectEnd_ = e->pos();
        QPointF tl = selectStart_;
        QPointF br = selectEnd_;
        if (tl.x() > br.x())
            std::swap(tl.rx(), br.rx());
        if (tl.y() > br.y())
            std::swap(tl.ry(), br.ry());
        double lat1, lon1, lat2, lon2;
        pixelToLonLat(tl + QPointF(0, 0), lat2, lon1);
        pixelToLonLat(br, lat1, lon2);
        currentBox_.minLat = lat1;
        currentBox_.maxLat = lat2;
        currentBox_.minLon = lon1;
        currentBox_.maxLon = lon2;
        emit boxChanged(currentBox_);
        update();
    }
}

void MapCanvas::wheelEvent(QWheelEvent* e)
{
    int delta = e->angleDelta().y() / 120;
    if (!delta)
        return;
    setZoom(zoom_ + delta);
}

QImage MapCanvas::renderSelection(int tileSize) const
{
    if (currentBox_.maxLat <= currentBox_.minLat || currentBox_.maxLon <= currentBox_.minLon)
        return QImage();
    QPointF tl = lonLatToPixel(currentBox_.maxLat, currentBox_.minLon);
    QPointF br = lonLatToPixel(currentBox_.minLat, currentBox_.maxLon);
    int w = int(br.x() - tl.x());
    int h = int(br.y() - tl.y());
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    int x0 = int(std::floor(tl.x() / tileSize));
    int y0 = int(std::floor(tl.y() / tileSize));
    int x1 = int(std::floor((br.x()) / tileSize));
    int y1 = int(std::floor((br.y()) / tileSize));
    for (int x = x0; x <= x1; ++x) {
        for (int y = y0; y <= y1; ++y) {
            int tx = (x % int(std::pow(2, zoom_)) + int(std::pow(2, zoom_))) % int(std::pow(2, zoom_));
            int ty = (y % int(std::pow(2, zoom_)) + int(std::pow(2, zoom_))) % int(std::pow(2, zoom_));
            QString key = QString("%1/%2/%3").arg(zoom_).arg(tx).arg(ty);
            QPixmap* pix = tileCache_.object(key);
            if (!pix)
                requestTile(tx, ty);
            if (pix)
                p.drawPixmap(x * tileSize - tl.x(), y * tileSize - tl.y(), *pix);
        }
    }
    return img;
}
