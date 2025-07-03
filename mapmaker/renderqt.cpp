#include <renderqt.h>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <iostream>
#include <sstream>
#include <memory>
#include <geos.h>
#include <numeric>
#include "linebreaking.h"
#include <geos/simplify/DouglasPeuckerLineSimplifier.h>
#include "textfield.h"

RenderQT::RenderQT(Project* project, int dpiScale)
{
    project_ = project;
    scale_ = dpiScale;
}

void RenderQT::SetupZoomAtCenter(int imageWithPixels, int imageHeightPixels, double centerX, double centerY, double pixelResolution)
{
    imageWithPixels_ = imageWithPixels;
    imageHeightPixels_ = imageHeightPixels;
    left_ = centerX - imageWithPixels / 2 * pixelResolution;
    right_ = centerX + imageWithPixels / 2 * pixelResolution;
    bottom_ = centerY - imageHeightPixels / 2 * pixelResolution;
    top_ = centerY + imageHeightPixels / 2 * pixelResolution;
}

void RenderQT::SetupZoomBoundingBox(int imageWithPixels, int imageHeightPixels, double left, double right, double bottom, double top)
{
    imageWithPixels_ = imageWithPixels;
    imageHeightPixels_ = imageHeightPixels;
    left_ = left;
    right_ = right;
    bottom_ = bottom;
    top_ = top;
}

QImage RenderQT::RenderImage()
{
    QImage img(imageWithPixels_, imageHeightPixels_, QImage::Format_ARGB32_Premultiplied);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor backgroundColor = project_->backgroundColor();
    painter.fillRect(0, 0, imageWithPixels_, imageHeightPixels_, backgroundColor);

    auto zoomToScale = std::map<int, double>();
    zoomToScale[0] = 1000000000 / scale_;
    zoomToScale[1] = 500000000 / scale_;
    zoomToScale[2] = 200000000 / scale_;
    zoomToScale[3] = 100000000 / scale_;
    zoomToScale[4] = 50000000 / scale_;
    zoomToScale[5] = 25000000 / scale_;
    zoomToScale[6] = 12500000 / scale_;
    zoomToScale[7] = 6500000 / scale_;
    zoomToScale[8] = 3000000 / scale_;
    zoomToScale[9] = 1500000 / scale_;
    zoomToScale[10] = 750000 / scale_;
    zoomToScale[11] = 400000 / scale_;
    zoomToScale[12] = 200000 / scale_;
    zoomToScale[13] = 100000 / scale_;
    zoomToScale[14] = 50000 / scale_;
    zoomToScale[15] = 25000 / scale_;
    zoomToScale[16] = 12500 / scale_;
    zoomToScale[17] = 5000 / scale_;
    zoomToScale[18] = 2500 / scale_;
    zoomToScale[19] = 1500 / scale_;
    zoomToScale[20] = 750 / scale_;
    zoomToScale[21] = 500 / scale_;
    zoomToScale[22] = 250 / scale_;
    zoomToScale[23] = 100 / scale_;
    zoomToScale[24] = 50 / scale_;
    zoomToScale[25] = 25 / scale_;
    zoomToScale[26] = 12.5 / scale_;

    RenderGeom(painter, zoomToScale);
    RenderLabels(painter, zoomToScale);

    return img;
}

void RenderQT::RenderGeom(QPainter& painter, std::map<int, double>& zoomToScale)
{
    QTransform transform1;
    transform1.translate(-left_, -top_);

    QTransform transform2;
    transform2.scale(imageWithPixels_ / (right_ - left_), -imageHeightPixels_ / (top_ - bottom_));

    double penScaleMPerPixel = (right_ - left_) / imageWithPixels_;

    QTransform t = transform1 * transform2;

    painter.setTransform(t);

    std::vector<StyleLayer*> layers = project_->styleLayers();

    SQLite::Database* db = project_->renderDatabase();

    geos::io::WKBReader geomFactory;

    for (auto projectLayerI = layers.rbegin(); projectLayerI != layers.rend(); ++projectLayerI) {
        StyleLayer* projectLayer = *projectLayerI;

        std::vector<QString> names = projectLayer->subLayerNames();

        bool layerActive = false;

        for (int subLayerIndex = 0; layerActive == false && subLayerIndex < names.size(); ++subLayerIndex) {
            StyleSelector selector = projectLayer->subLayerSelectors(subLayerIndex);

            switch (projectLayer->layerType()) {
            case ST_POINT: {
                auto point = projectLayer->subLayerPoint(subLayerIndex);
                if (point.visible_ == false)
                    continue;

                double minZoomScale = zoomToScale[point.minZoom_];
                double currentScale = penScaleMPerPixel / 0.00028;
                if (currentScale > minZoomScale)
                    continue;

                layerActive = true;

                break;
            }

            case ST_LINE: {
                Line line = projectLayer->subLayerLine(subLayerIndex);
                if (line.visible_ == false)
                    continue;

                double minZoomScale = zoomToScale[line.minZoom_];
                double currentScale = penScaleMPerPixel / 0.00028;
                if (currentScale > minZoomScale)
                    continue;

                layerActive = true;

                break;
            }

            case ST_AREA: {
                Area area = projectLayer->subLayerArea(subLayerIndex);
                if (area.visible_ == false)
                    continue;

                double minZoomScale = zoomToScale[area.minZoom_];
                double currentScale = penScaleMPerPixel / 0.00028;
                if (currentScale > minZoomScale)
                    continue;

                layerActive = true;
            }
            }
        }

        if (layerActive) {
            std::string layerSQL = projectLayer->renderSQLSelect(false).toStdString();
            SQLite::Statement query(*db, layerSQL);

            while (query.executeStep()) {
                std::vector<std::string> selectionValues;

                bool subLayerRendered = false;
                for (int subLayerIndex = 0; subLayerRendered == false && subLayerIndex < names.size(); ++subLayerIndex) {
                    const StyleSelector& selector = projectLayer->subLayerSelectors(subLayerIndex);

                    subLayerRendered = true;

                    for (size_t c = 0; subLayerRendered && c < selector.conditionCount(); ++c) {
                        std::string key;
                        selector.condition(c, &key, &selectionValues);

                        SQLite::Column col = query.getColumn(key.c_str());
                        std::string colVal = col.getString();

                        subLayerRendered = false;

                        for (auto v : selectionValues) {
                            if (colVal == v || v == "*") {
                                subLayerRendered = true;
                                break;
                            }
                        }
                    }

                    if (subLayerRendered) {
                        SQLite::Column col = query.getColumn("geom");

                        std::istringstream strStr(col.getString());

                        std::unique_ptr<geos::geom::Geometry> geom = geomFactory.read(strStr);

                        struct toMap : public CoordinateFilter {
                            PJ* mapToData;

                            void filter_rw(Coordinate* coord) const
                            {
                                PJ_COORD in;
                                memset(&in, 0, sizeof(in));
                                in.lp.lam = coord->x;
                                in.lp.phi = coord->y;

                                PJ_COORD out = proj_trans(mapToData, PJ_FWD, in);

                                coord->x = out.xyzt.x;
                                coord->y = out.xyzt.y;
                            }
                        };

                        toMap tm;
                        tm.mapToData = project_->dataToMap_;
                        geom->apply_rw(&tm);

                        switch (projectLayer->layerType()) {
                        case ST_POINT: {
                            auto point = projectLayer->subLayerPoint(subLayerIndex);

                            if (point.width_ > 0) {
                                QColor c = point.color_;
                                c.setAlphaF(point.opacity_);
                                QBrush brush(c);
                                painter.setBrush(brush);

                                std::unique_ptr<geos::geom::Point> location = geom->getCentroid();
                                painter.drawEllipse(QPointF(location->getX(), location->getY()), point.width_ * penScaleMPerPixel / 2, penScaleMPerPixel * point.width_ / 2);
                            }

                            break;
                        }

                        case ST_LINE: {
                            Line line = projectLayer->subLayerLine(subLayerIndex);
                            if (line.visible_ == false)
                                continue;

                            double minZoomScale = zoomToScale[line.minZoom_];
                            double currentScale = penScaleMPerPixel / 0.00028;
                            if (currentScale > minZoomScale)
                                continue;

                            geos::geom::LineString* ls = dynamic_cast<geos::geom::LineString*>(geom.get());

                            QPolygonF poly;
                            poly.reserve(ls->getNumPoints());
                            for (int n = 0; n < ls->getNumPoints(); ++n) {
                                const Coordinate& c = ls->getCoordinateN(n);
                                poly.append(QPointF(c.x, c.y));
                            }

                            if (line.casingWidth_ > 0) {
                                QColor c = line.casingColor_;
                                c.setAlphaF(line.opacity_);
                                QBrush brush(c);
                                QPen pen(brush,
                                    (line.width_ + line.casingWidth_) * 2.0 * scale_ * penScaleMPerPixel,
                                    Qt::SolidLine,
                                    Qt::FlatCap,
                                    Qt::BevelJoin);

                                painter.setPen(pen);
                                painter.drawPolyline(poly);
                            }

                            QColor c = line.color_;
                            c.setAlphaF(line.opacity_);
                            QBrush brush(c);
                            QPen pen(brush,
                                line.width_ * scale_ * penScaleMPerPixel,
                                Qt::SolidLine,
                                Qt::FlatCap,
                                Qt::BevelJoin);

                            if (line.dashArray_.size() > 0) {
                                QVector<qreal> dashs;
                                for (auto d : line.dashArray_) {
                                    dashs.push_back(d.first / line.width_);
                                    dashs.push_back(d.second / line.width_);
                                }

                                pen.setDashPattern(dashs);
                            }

                            painter.setPen(pen);

                            painter.drawPolyline(poly);

                            break;
                        }

                        case ST_AREA: {
                            Area area = projectLayer->subLayerArea(subLayerIndex);
                            if (area.visible_ == false)
                                continue;

                            double minZoomScale = zoomToScale[area.minZoom_];
                            double currentScale = penScaleMPerPixel / 0.00028;
                            if (currentScale > minZoomScale)
                                continue;

                            geos::geom::MultiPolygon* poly = dynamic_cast<geos::geom::MultiPolygon*>(geom.get());

                            for (const auto& polyG_ptr : *poly) {
                                geos::geom::Polygon* poly = dynamic_cast<geos::geom::Polygon*>(polyG_ptr.get());

                                const LineString* polyOuter = poly->getExteriorRing();

                                QPolygonF outerP;

                                for (int n = 0; n < polyOuter->getNumPoints(); ++n) {
                                    const Coordinate& c = polyOuter->getCoordinateN(n);
                                    outerP.append(QPointF(c.x, c.y));
                                }

                                QPainterPath areaP;
                                areaP.addPolygon(outerP);

                                for (int innerRingIndex = 0; innerRingIndex < poly->getNumInteriorRing(); ++innerRingIndex) {
                                    const LineString* holeG = poly->getInteriorRingN(innerRingIndex);

                                    QPolygonF hole;
                                    for (int n = 0; n < holeG->getNumPoints(); ++n) {
                                        const Coordinate& c = holeG->getCoordinateN(n);
                                        hole.append(QPointF(c.x, c.y));
                                    }

                                    QPainterPath holeP;
                                    holeP.addPolygon(hole);

                                    areaP = areaP.subtracted(holeP);
                                }

                                if (area.opacity_ > 0) {
                                    QColor c = area.color_;
                                    c.setAlphaF(area.opacity_);
                                    QBrush brush(c);
                                    painter.setBrush(brush);
                                    painter.setPen(QPen(Qt::NoPen));
                                    painter.drawPath(areaP);
                                }

                                if (area.fillImage_.isEmpty() == false) {
                                }

                                if (area.casingWidth_ > 0) {
                                    QColor c = area.casingColor_;
                                    c.setAlphaF(area.opacity_);
                                    QBrush brush(c);
                                    QPen pen(brush,
                                        area.casingWidth_ * scale_ * penScaleMPerPixel,
                                        Qt::SolidLine,
                                        Qt::FlatCap,
                                        Qt::BevelJoin);

                                    painter.setPen(pen);

                                    painter.drawPolyline(outerP);
                                }
                            }

                            break;
                        }
                        }
                    }
                }
            }
        }
    }
}

void RenderQT::RenderLabels(QPainter& painter, std::map<int, double>& zoomToScale)
{
    double penScaleMPerPixel = (right_ - left_) / imageWithPixels_;

    painter.resetTransform();

    std::vector<StyleLayer*> layers = project_->styleLayers();

    SQLite::Database* db = project_->renderDatabase();

    geos::io::WKBReader geomFactoryWKB;

    GeometryFactory::Ptr geomFactory = GeometryFactory::create();

    QFont normalFont("Arial", -1, QFont::Normal, false);
    QFont lightFont("Arial", -1, QFont::Light, false);
    QFont heavyFont("Arial", -1, QFont::Bold, false);

    for (auto projectLayerI = layers.rbegin(); projectLayerI != layers.rend(); ++projectLayerI) {
        StyleLayer* projectLayer = *projectLayerI;

        std::vector<QString> names = projectLayer->subLayerNames();

        std::vector<int> order(names.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int a, int b) { return projectLayer->label(a).priority_ < projectLayer->label(b).priority_; });

        bool layerActive = false;

        for (size_t orderIndex = 0; layerActive == false && orderIndex < order.size(); ++orderIndex) {
            int subLayerIndex = order[orderIndex];
            Label label = projectLayer->label(subLayerIndex);

            if (label.mapnikText().isEmpty())
                continue;

            StyleSelector selector = projectLayer->subLayerSelectors(subLayerIndex);

            switch (projectLayer->layerType()) {
            case ST_POINT: {
                auto point = projectLayer->subLayerPoint(subLayerIndex);
                if (point.visible_ == false)
                    continue;

                double minZoomScale = zoomToScale[point.minZoom_];
                double currentScale = penScaleMPerPixel / 0.00028;
                if (currentScale > minZoomScale)
                    continue;

                minZoomScale = zoomToScale[label.minZoom_];
                if (currentScale > minZoomScale)
                    continue;

                layerActive = true;

                break;
            }

            case ST_LINE: {
                Line line = projectLayer->subLayerLine(subLayerIndex);
                if (line.visible_ == false)
                    continue;

                double minZoomScale = zoomToScale[line.minZoom_];
                double currentScale = penScaleMPerPixel / 0.00028;
                if (currentScale > minZoomScale)
                    continue;

                minZoomScale = zoomToScale[label.minZoom_];
                if (currentScale > minZoomScale)
                    continue;

                layerActive = true;

                break;
            }

            case ST_AREA: {
                Area area = projectLayer->subLayerArea(subLayerIndex);
                if (area.visible_ == false)
                    continue;

                double minZoomScale = zoomToScale[area.minZoom_];
                double currentScale = penScaleMPerPixel / 0.00028;
                if (currentScale > minZoomScale)
                    continue;

                minZoomScale = zoomToScale[label.minZoom_];
                if (currentScale > minZoomScale)
                    continue;

                layerActive = true;
            }
            }
        }

        if (layerActive) {
            std::string layerSQL = projectLayer->renderSQLSelect(false).toStdString();
            SQLite::Statement query(*db, layerSQL);

            while (query.executeStep()) {
                std::vector<std::string> selectionValues;

                bool subLayerRendered = false;
                for (size_t orderIndex = 0; subLayerRendered == false && orderIndex < order.size(); ++orderIndex) {
                    int subLayerIndex = order[orderIndex];
                    const StyleSelector& selector = projectLayer->subLayerSelectors(subLayerIndex);

                    Label label = projectLayer->label(subLayerIndex);

                    subLayerRendered = true;

                    for (size_t c = 0; subLayerRendered && c < selector.conditionCount(); ++c) {
                        std::string key;
                        selector.condition(c, &key, &selectionValues);

                        SQLite::Column col = query.getColumn(key.c_str());
                        std::string colVal = col.getString();

                        subLayerRendered = false;

                        for (auto v : selectionValues) {
                            if (colVal == v || v == "*") {
                                subLayerRendered = true;
                                break;
                            }
                        }
                    }

                    if (subLayerRendered) {
                        SQLite::Column col = query.getColumn("geom");

                        std::istringstream strStr(col.getString());

                        std::unique_ptr<geos::geom::Geometry> geom = geomFactoryWKB.read(strStr);

                        struct toMap : public CoordinateFilter {
                            PJ* mapToData;

                            void filter_rw(Coordinate* coord) const
                            {
                                PJ_COORD in;
                                memset(&in, 0, sizeof(in));
                                in.lp.lam = coord->x;
                                in.lp.phi = coord->y;

                                PJ_COORD out = proj_trans(mapToData, PJ_FWD, in);

                                coord->x = out.xyzt.x;
                                coord->y = out.xyzt.y;
                            }
                        };

                        toMap tm;
                        tm.mapToData = project_->dataToMap_;
                        geom->apply_rw(&tm);

                        QString labelTextUser = label.mapnikText();
                        QString labelText;
                        if (label.visible_ && labelTextUser.isEmpty() == false) {
                            if (labelTextUser == "[name] [ref]")
                                labelTextUser = labelTextUser;

                            std::map<QString, QString> keys;
                            for (int cc = 0; cc < query.getColumnCount(); ++cc) {
                                QString col = query.getColumnName(cc);
                                QString val = QString::fromStdString(query.getColumn(cc).getString());
                                keys.insert(std::pair<QString, QString>(col, val));
                            }

                            labelText = TextFieldProcessor::Expand(labelTextUser, keys);
                        }

                        double minZoomScale = zoomToScale[label.minZoom_];
                        double currentScale = penScaleMPerPixel / 0.00028;
                        if (currentScale > minZoomScale)
                            continue;

                        if (label.visible_ == false || labelText.isEmpty())
                            continue;

                        QFont normalFont;
                        QFont lightFont;
                        QFont heavyFont;

                        QFont* font = NULL;
                        if (label.fontWeight == 200)
                            font = &lightFont;
                        else if (label.fontWeight == 700)
                            font = &heavyFont;
                        else
                            font = &normalFont;

                        font->setPixelSize(label.height_ * scale_);

                        QFontMetricsF metrics(*font);

                        double xScale = imageWithPixels_ / (right_ - left_);
                        double yScale = imageHeightPixels_ / (top_ - bottom_);

                        switch (projectLayer->layerType()) {
                        case ST_POINT: {
                            auto point = projectLayer->subLayerPoint(subLayerIndex);

                            if (point.visible_ == false)
                                continue;

                            double minZoomScale = zoomToScale[point.minZoom_];
                            double currentScale = penScaleMPerPixel / 0.00028;
                            if (currentScale > minZoomScale)
                                continue;

                            minZoomScale = zoomToScale[label.minZoom_];
                            if (currentScale > minZoomScale)
                                continue;

                            std::unique_ptr<geos::geom::Point> center = geom->getCentroid();

                            float x = (center->getX() - left_) * xScale;
                            float y = (top_ - center->getY()) * yScale;

                            painter.resetTransform();
                            painter.translate(x, y);

                            QPainterPath path;

                            painter.setPen(QPen(Qt::NoPen));

                            QSizeF textWidthPixels = metrics.size(Qt::TextSingleLine, labelText);

                            path.addText(-textWidthPixels.width() / 2, label.offsetY_, *font, labelText);

                            if (label.haloSize_ > 0) {
                                QPainterPathStroker halo;
                                halo.setWidth(label.haloSize_ * 2 * scale_);
                                painter.setBrush(label.haloColor_);
                                painter.drawPath(halo.createStroke(path));
                            }

                            painter.setBrush(label.color_);
                            painter.drawPath(path);

                        } break;

                        case ST_LINE: {
                            Line line = projectLayer->subLayerLine(subLayerIndex);

                            if (line.visible_ == false)
                                continue;

                            double minZoomScale = zoomToScale[line.minZoom_];
                            double currentScale = penScaleMPerPixel / 0.00028;
                            if (currentScale > minZoomScale)
                                continue;

                            minZoomScale = zoomToScale[label.minZoom_];
                            if (currentScale > minZoomScale)
                                continue;

                            std::unique_ptr<CoordinateSequence> cs = geom->getCoordinates();

                            geos::simplify::DouglasPeuckerLineSimplifier s(*cs);
                            s.setDistanceTolerance(5.0 / xScale);

                            std::unique_ptr<CoordinateSequence> newCoords = s.simplify();

                            std::unique_ptr<LineString> lineString = geomFactory->createLineString(std::move(newCoords));

                            std::size_t nPoints = lineString->getNumPoints();

                            double length = geom->getLength();

                            QSizeF textWidthPixels = metrics.size(Qt::TextSingleLine, labelText);

                            // length in pixels
                            double lengthPixels = 0;
                            for (int i = 1; i < nPoints; ++i) {
                                std::unique_ptr<geos::geom::Point> pt1 = lineString->getPointN(i - 1);
                                std::unique_ptr<geos::geom::Point> pt2 = lineString->getPointN(i);

                                double pt1X = pt1->getX();
                                double pt1Y = pt1->getY();

                                double pt2X = pt2->getX();
                                double pt2Y = pt2->getY();

                                double dX = pt2X - pt1X;
                                double dY = pt2Y - pt1Y;

                                double deltaRatio = 0;

                                bool emitText = false;

                                double distPixels = sqrt(dX * dX * xScale * xScale + dY * dY * yScale * yScale);
                                lengthPixels += distPixels;
                            }

                            double labelMaxGap = label.maxWrapWidth_;
                            if (labelMaxGap <= 0)
                                labelMaxGap = 400;

                            // spacing specified
                            double maxPitch = textWidthPixels.width() + labelMaxGap;
                            double numberOfLabelGaps = ceil(lengthPixels / maxPitch);
                            if (numberOfLabelGaps <= 1)
                                numberOfLabelGaps = 2.0;

                            double pitch = lengthPixels / numberOfLabelGaps;

                            if (pitch < textWidthPixels.width() / 2.2)
                                break; // too small don't render it.

                            double t = 0;
                            double lengthTotal = 0;
                            double nextLabel = pitch;
                            for (int i = 1; i < nPoints; ++i) {
                                std::unique_ptr<geos::geom::Point> pt1 = lineString->getPointN(i - 1);
                                std::unique_ptr<geos::geom::Point> pt2 = lineString->getPointN(i);

                                double pt1X = pt1->getX();
                                double pt1Y = pt1->getY();

                                double pt2X = pt2->getX();
                                double pt2Y = pt2->getY();

                                double dX = pt2X - pt1X;
                                double dY = pt2Y - pt1Y;

                                double segmentLengthPixels = sqrt(dX * dX * xScale * xScale + dY * dY * yScale * yScale);

                                while (lengthTotal + segmentLengthPixels > nextLabel) {
                                    double usedPixels = nextLabel - lengthTotal;

                                    double deltaRatio = usedPixels / segmentLengthPixels;

                                    segmentLengthPixels -= usedPixels;
                                    lengthTotal += usedPixels;
                                    nextLabel += pitch;

                                    QPainterPath path;

                                    double centerX = pt1X + (deltaRatio * dX);
                                    double centerY = pt1Y + (deltaRatio * dY);

                                    float x = (centerX - left_) * xScale;
                                    float y = (top_ - centerY) * yScale;

                                    painter.resetTransform();
                                    painter.translate(x, y);
                                    double rotAngleDeg = atan2(dY, dX) * -180.0 / 3.141596;
                                    if (rotAngleDeg > 90)
                                        rotAngleDeg -= 180;
                                    else if (rotAngleDeg < -90)
                                        rotAngleDeg += 180;

                                    painter.rotate(rotAngleDeg);

                                    path.addText(-textWidthPixels.width() / 2, label.offsetY_, *font, labelText);

                                    painter.setPen(QPen(Qt::NoPen));

                                    if (label.haloSize_ > 0) {
                                        QPainterPathStroker halo;
                                        halo.setWidth(label.haloSize_ * 2 * scale_);
                                        painter.setBrush(label.haloColor_);
                                        painter.drawPath(halo.createStroke(path));
                                    }

                                    painter.setBrush(label.color_);
                                    painter.drawPath(path);
                                }

                                lengthTotal += segmentLengthPixels;
                            }
                        } break;

                        case ST_AREA: {
                            Area area = projectLayer->subLayerArea(subLayerIndex);
                            if (area.visible_ == false)
                                continue;

                            double minZoomScale = zoomToScale[area.minZoom_];
                            double currentScale = penScaleMPerPixel / 0.00028;
                            if (currentScale > minZoomScale)
                                continue;

                            minZoomScale = zoomToScale[label.minZoom_];
                            if (currentScale > minZoomScale)
                                continue;

                            painter.resetTransform();

                            QPainterPath path;

                            std::unique_ptr<geos::geom::Point> center = geom->getCentroid();

                            QSizeF textWidthPixels = metrics.size(Qt::TextSingleLine, labelText);
                            const double goldenRatio = 1.618;

                            double maxWrapWidthPixels = textWidthPixels.width();
                            double bestRatio = 1e100;
                            for (double ii = 1; ii < 10; ++ii) {
                                double ratio = (textWidthPixels.width() / ii) / (textWidthPixels.height() * ii);
                                double ratioError = (ratio / goldenRatio) - 1.0;

                                // too small is not good, punish too small.
                                if (ratioError < 0)
                                    ratioError = ratioError * -5.0;

                                if (ratioError < bestRatio) {
                                    maxWrapWidthPixels = textWidthPixels.width() / ii;
                                    bestRatio = ratioError;
                                }
                            }

                            bool tooLarge = false;

                            const Envelope* geomBB = geom->getEnvelopeInternal();

                            double geomWidthPixels = geomBB->getWidth() * xScale;

                            QTextBoundaryFinder wordFinder(QTextBoundaryFinder::Word, labelText);

                            std::vector<QString> words;
                            std::vector<int> wordLengths;
                            int longestWord = 0;
                            int lastPos = 0;
                            while (wordFinder.toNextBoundary() > 0) {
                                int pos = wordFinder.position();

                                if (labelText[pos].isSpace() == false) {
                                    QString word = labelText.mid(lastPos, pos - lastPos);
                                    words.push_back(word);
                                    int wordSize = metrics.size(Qt::TextSingleLine, word).width();
                                    if (wordSize > longestWord)
                                        longestWord = wordSize;
                                    wordLengths.push_back(wordSize);
                                    lastPos = pos;
                                    if (wordLengths.back() > geomWidthPixels)
                                        tooLarge = true;
                                }
                            }

                            if (longestWord < geomWidthPixels) {
                                double maxWidthPixels = std::min(maxWrapWidthPixels + longestWord / 2, geomWidthPixels);

                                std::vector<size_t> breaks;
                                breakLines(wordLengths, maxWidthPixels, &breaks);

                                float x = (center->getX() - left_) * xScale;
                                float y = (top_ - center->getY()) * yScale;

                                QString lineText;
                                size_t nextBreakIndex = 0;
                                int lineCount = 0;
                                for (int i = 0; i < words.size(); ++i) {
                                    lineText += words[i];
                                    if ((i + 1 == words.size()) || (nextBreakIndex < breaks.size() && breaks[nextBreakIndex] == i + 1)) {
                                        QSizeF lineWidthPixels = metrics.size(Qt::TextSingleLine, lineText);

                                        path.addText(x - lineWidthPixels.width() / 2, y - textWidthPixels.height() / 2, *font, lineText);
                                        y += textWidthPixels.height();
                                        ++lineCount;
                                        ++nextBreakIndex;
                                        lineText.clear();
                                    }
                                }

                                painter.setPen(QPen(Qt::NoPen));

                                if (label.haloSize_ > 0) {
                                    QPainterPathStroker halo;
                                    halo.setWidth(label.haloSize_ * 2 * scale_);
                                    painter.setBrush(label.haloColor_);
                                    painter.drawPath(halo.createStroke(path));
                                }

                                painter.setBrush(label.color_);
                                painter.drawPath(path);
                            }
                        } break;
                        }
                    }
                }
            }
        }
    }
}
