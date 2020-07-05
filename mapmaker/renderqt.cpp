#include <renderqt.h>
#include <QPainter>
#include <iostream>     
#include <sstream>
#include <geos/geos.h>
#include <proj.h>

RenderQT::RenderQT(Project *project, int dpiScale)
{
	project_ = project;
	scale_ = dpiScale;

}

void RenderQT::SetupZoomAtCenter(int imageWithPixels, int imageHeightPixels, double centerX, double centerY, double pixelResolution)
{
	imageWithPixels_ = imageWithPixels;
	imageHeightPixels_ = imageHeightPixels;
	left_ = centerX - imageWithPixels/2* pixelResolution;
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
	QImage img(imageWithPixels_, imageHeightPixels_, QImage::Format_ARGB32);

	QPainter painter(&img);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QColor backgroundColor = project_->backgroundColor();
	painter.fillRect(0, 0, imageWithPixels_, imageHeightPixels_, backgroundColor);

	QTransform transform1;
	transform1.translate(-left_, -top_);

	QTransform transform2;
	transform2.scale(imageWithPixels_ / (right_ - left_), -imageHeightPixels_ / (top_ - bottom_));

	double penScaleMPerPixel = (right_ - left_) / imageWithPixels_;

	QTransform t = transform1 * transform2;

	QPointF llPixel = t.map(QPointF(left_, bottom_));
	QPointF urPixel = t.map(QPointF(right_, top_));

	painter.setTransform(t);

	std::vector< StyleLayer*> layers = project_->styleLayers();

	SQLite::Database *db = project_->renderDatabase();

	geos::io::WKBReader geomFactory;

	PJ_CONTEXT *proj_context = proj_context_create();

	PJ *mapToData = proj_create_crs_to_crs(proj_context, project_->dataSRS().c_str(), project_->mapSRS().c_str(), NULL);

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

	for (auto projectLayerI = layers.rbegin(); projectLayerI != layers.rend(); ++projectLayerI)
	{
		StyleLayer *projectLayer = *projectLayerI;

		std::string layerSQL = projectLayer->renderSQLSelect(false).toStdString();
		SQLite::Statement query(*db, layerSQL);

		std::vector<QString> names = projectLayer->subLayerNames();

		while (query.executeStep())
		{
			std::vector<QString> selectionValues;

			bool subLayerRendered = false;
			for (int subLayerIndex = 0; subLayerRendered == false && subLayerIndex < names.size(); ++subLayerIndex)
			{
				StyleSelector selector = projectLayer->subLayerSelectors(subLayerIndex);

				subLayerRendered = true;

				for (size_t c = 0; subLayerRendered && c < selector.conditionCount(); ++c)
				{
					QString key;
					selector.condition(c, &key, &selectionValues);


					SQLite::Column col = query.getColumn(key.toStdString().c_str());
					QString colVal = QString::fromStdString(col.getString());

					if (key == "highway" && colVal == "service")
						key = key;

					subLayerRendered = false;

					for (auto v : selectionValues)
					{
						if (colVal == v)
						{
							subLayerRendered = true;
							break;
						}
					}


				}

				if (subLayerRendered)
				{
					SQLite::Column col = query.getColumn("geom");

					std::istringstream strStr(col.getString());

					geos::geom::Geometry *geom = geomFactory.read(strStr);

					struct toMap : public CoordinateFilter
					{
						PJ *mapToData;

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
					tm.mapToData = mapToData;
					geom->apply_rw(&tm);

					switch (projectLayer->layerType())
					{
					case ST_POINT:
					{
						break;
					}

					case ST_LINE:
					{
						Line line = projectLayer->subLayerLine(subLayerIndex);
						if (line.visible_ == false)
							continue;

						double minZoomScale = zoomToScale[line.minZoom_];
						double currentScale = penScaleMPerPixel / 0.00028;
						if (currentScale > minZoomScale)
							continue;

						geos::geom::LineString *ls = dynamic_cast<geos::geom::LineString*>(geom);

						QPolygonF poly;
						poly.reserve(ls->getNumPoints());
						for (int n = 0; n < ls->getNumPoints(); ++n)
						{
							const Coordinate &c = ls->getCoordinateN(n);
							poly.append(QPointF(c.x, c.y));
						}

						if (line.casingWidth_ > 0)
						{
							QColor c = line.casingColor_;
							c.setAlphaF(line.opacity_);
							QBrush brush(c);
							QPen pen(brush,
								(line.width_ + line.casingWidth_)*2.0*scale_*penScaleMPerPixel,
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


						if (line.dashArray_.size() > 0)
						{
							QVector<qreal> dashs;
							for (auto d : line.dashArray_)
							{
								dashs.push_back(d.first / line.width_);
								dashs.push_back(d.second / line.width_);
							}

							pen.setDashPattern(dashs);
						}

						painter.setPen(pen);

						painter.drawPolyline(poly);

						break;
					}

					case ST_AREA:
					{
						Area area = projectLayer->subLayerArea(subLayerIndex);
						if (area.visible_ == false)
							continue;

						double minZoomScale = zoomToScale[area.minZoom_];
						double currentScale = penScaleMPerPixel / 0.00028;
						if (currentScale > minZoomScale)
							continue;

						geos::geom::MultiPolygon *poly = dynamic_cast<geos::geom::MultiPolygon *>(geom);


						for (geos::geom::Geometry *polyG : *poly)
						{
							geos::geom::Polygon *poly = dynamic_cast<geos::geom::Polygon*>(polyG);

							const LineString* polyOuter = poly->getExteriorRing();

							QPolygonF outerP;

							for (int n = 0; n < polyOuter->getNumPoints(); ++n)
							{
								const Coordinate &c = polyOuter->getCoordinateN(n);
								outerP.append(QPointF(c.x, c.y));
							}
							

							if (area.opacity_ > 0)
							{
								QColor c = area.color_;
								c.setAlphaF(area.opacity_);
								QBrush brush(c);
								painter.setBrush(brush);
								painter.drawPolygon(outerP);
							}

							if (area.fillImage_.isEmpty() == false)
							{
							}

							if (area.casingWidth_ > 0)
							{
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

	return img;
}
