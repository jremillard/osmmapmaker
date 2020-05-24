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
	StyleSelector(const QString &key);
	StyleSelector();
	~StyleSelector();

	QString mapniKExpression();

	void clear();
	size_t conditionCount() const;
	void setCondition(size_t i, const QString &key, const std::vector<QString> &values);
	void condition(size_t i, QString *key, std::vector<QString> *values) const;

	void insertCondition(size_t i, const QString &key, const std::vector<QString> &values);
	void deleteCondition(size_t i);


private:
	std::vector<QString> keys_;
	std::vector< std::vector<QString>> values_;
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

class Label
{
public:
	Label();

	bool visible_;
	QString text_;
	double height_;
	QColor color_;
	double haloSize_;
	QColor haloColor_;
	double lineLaxSpacing_;
	double maxWrapWidth_;
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

	StyleSelector subLayerSelectors(size_t i);
	void setSubLayerSelectors(size_t i, const StyleSelector &selections);

	Line subLayerLine(size_t i);
	void setSubLayerLine(size_t i, const Line &lines);

	Area subLayerArea(size_t i);
	void setSubLayerArea(size_t i, const Area &area);

	Label label(size_t i);
	void setLabel(size_t i, const Label &lb);

	void showAll();
	void hideAll();

	std::vector<QString> requiredKeys();

private:
	StyleLayerType type_;
	QString key_;
	QString dataSource_;

	std::vector<Line > lines_;
	std::vector<Area> areas_;
	std::vector<Label> labels_;
	std::vector<StyleSelector> selectors_;
};

