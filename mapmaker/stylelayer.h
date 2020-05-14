#pragma once

#include <QString>
#include <vector>
#include <QColor>
#include <QtXml>

enum StyleLayerType
{
	ST_LABEL,
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


class StyleLayer
{
public:
	StyleLayer(QString key, StyleLayerType type);
	StyleLayer(QDomElement layerNode);
	virtual ~StyleLayer();

	QString key();

	void saveXML(QDomDocument &doc, QDomElement &layerElement);

	StyleLayerType layerType();

	std::vector<QString> subLayerNames();

	std::vector<StyleSelector> subLayerSelectors(size_t i);
	void setSubLayerSelectors(size_t i, const std::vector<StyleSelector> &selections);

	Line subLayerLine(size_t i);
	void setSubLayerLine(size_t i, const Line &lines);

private:
	StyleLayerType type_;
	QString key_;

	std::vector<Line > lines_;
	std::vector<std::vector<StyleSelector>  > selectors_;
};
