#pragma once
#include <QWidget>
#include <QCache>
#include <QNetworkAccessManager>
#include <QSet>
#include <QImage>
#include "maputils.h"

struct BoundingBoxCoords {
    double minLat;
    double minLon;
    double maxLat;
    double maxLon;
};

class MapCanvas : public QWidget {
    Q_OBJECT
public:
    explicit MapCanvas(QWidget* parent = nullptr);

    void setCenter(double lat, double lon);
    void setZoom(int z);
    int zoom() const { return zoom_; }
    BoundingBoxCoords selection() const { return currentBox_; }

    QImage renderSelection(int tileSize = 256) const;

signals:
    void boxChanged(const BoundingBoxCoords&);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    QPointF lonLatToPixel(double lat, double lon) const;
    void pixelToLonLat(const QPointF& p, double& lat, double& lon) const;
    void requestTile(int x, int y) const;

    mutable QCache<QString, QPixmap> tileCache_;
    mutable QSet<QString> pending_;
    mutable QNetworkAccessManager nam_;

    int zoom_ = 2;
    double centerLat_ = 0.0;
    double centerLon_ = 0.0;

    bool panning_ = false;
    QPoint panStart_;
    double panLatStart_ = 0.0;
    double panLonStart_ = 0.0;

    bool selecting_ = false;
    QPoint selectStart_;
    QPoint selectEnd_;
    BoundingBoxCoords currentBox_;
};
