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
	void condition(size_t i, std::string *key, std::vector<std::string> *values) const;

	void insertCondition(size_t i, const QString &key, const std::vector<QString> &values);
	void deleteCondition(size_t i);


private:
	std::vector<QString> keys_;
	std::vector< std::vector<QString>> values_;

	std::vector<std::string> keysStd_;
	std::vector< std::vector<std::string>> valuesStd_;

};

class SubLayer
{
public:
	SubLayer();
	virtual ~SubLayer();

	QString name_;

	bool visible_;
	QColor color_;
	int minZoom_;
	double opacity_;

};

class Point: public SubLayer
{
public:
	Point();

	QString image_;
	double width_;
};

class Line : public SubLayer
{
public:
	Line();

	QColor casingColor_;
	double width_;
	double casingWidth_;
	double smooth_;
	std::vector< std::pair<double, double> > dashArray_;
};

class Area : public SubLayer
{
public:
	Area();

	double casingWidth_;
	QColor casingColor_;
	QString fillImage_;
	double fillImageOpacity_;
};

class Label : public SubLayer
{
public:
	Label();

	QString mapnikText();

	int fontWeight;
	QString text_;
	double height_;
	double haloSize_;
	QColor haloColor_;
	double maxWrapWidth_;
	double offsetY_;
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

	StyleLayerType layerType() const;
	OsmEntityType dataType() const;

	std::vector<QString> subLayerNames() const;

	size_t subLayerCount() const;

	void removeSubLayer(size_t i);
	void subLayerMove(size_t i, int offset);

	const StyleSelector& subLayerSelectors(size_t i);
	void setSubLayerSelectors(size_t i, const StyleSelector &selections);

	Line subLayerLine(size_t i);
	void setSubLayerLine(size_t i, const Line &lines);

	Area subLayerArea(size_t i);
	void setSubLayerArea(size_t i, const Area &area);

	Point subLayerPoint(size_t i);
	void setSubLayerPoint(size_t i, const Point &area);

	Label label(size_t i);
	void setLabel(size_t i, const Label &lb);

	void showAll();
	void hideAll();

	std::vector<QString> requiredKeys() const;

	QString virtualSQLTableName() const;
	QString renderSQLSelect(bool sortBySize) const;


private:
	StyleLayerType type_;
	QString key_;
	QString dataSource_;

	std::vector<Point> points_;
	std::vector<Line > lines_;
	std::vector<Area> areas_;
	std::vector<Label> labels_;
	std::vector<StyleSelector> selectors_;
};

