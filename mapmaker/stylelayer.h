#pragma once

#include <QString>
#include <vector>
#include <QColor>
#include <QtXml>

#include "datasource.h"

enum StyleLayerType
{
	ST_POINT,
	ST_LINE,
	ST_AREA
};

class StyleSelector
{
public:
	StyleSelector();
	~StyleSelector();

	void GetAnds(std::vector<QString> *keys, std::vector<QString> *values);
	void SetAnds(const std::vector<QString> &keys, const std::vector<QString> &values);

private:
	std::vector<QString> keys_;
	std::vector<QString> values_;
};


class Line
{
public:
	Line();

	QString name_;
	bool visible_;
	QColor color_;
	QColor casingColor_;
	double width_;
	double casingWidth_;
	double opacity_;
	double smooth_;
};

class Area
{
public:
	Area();

	QString name_;
	bool visible_;
	QColor color_;
	double opacity_;

	double casingWidth_;
	QColor casingColor_;
	QString fillImage_;
	double fillImageOpacity_;
};

class StyleLayer
{
public:
	StyleLayer(QString dataSource, QString key, StyleLayerType type);
	StyleLayer(QDomElement layerNode);
	virtual ~StyleLayer();

	QString key();
	QString dataSource();

	void saveXML(QDomDocument &doc, QDomElement &layerElement);

	StyleLayerType layerType();
	OsmEntityType dataType();

	std::vector<QString> subLayerNames();

	std::vector<StyleSelector> subLayerSelectors(size_t i);
	void setSubLayerSelectors(size_t i, const std::vector<StyleSelector> &selections);

	Line subLayerLine(size_t i);
	void setSubLayerLine(size_t i, const Line &lines);

	Area subLayerArea(size_t i);
	void setSubLayerArea(size_t i, const Area &area);

	void showAll();
	void hideAll();

private:
	StyleLayerType type_;
	QString key_;
	QString dataSource_;

	std::vector<Line > lines_;
	std::vector<Area> areas_;
	std::vector<std::vector<StyleSelector>  > selectors_;
};
