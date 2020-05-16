#include "stylelayer.h"


StyleSelector::StyleSelector()
{
}

StyleSelector::~StyleSelector()
{
}

void StyleSelector::GetAnds(std::vector<QString> *keys, std::vector<QString> *values)
{
	*keys = keys_;
	*values= values_;
}

void StyleSelector::SetAnds(const std::vector<QString> &keys, const std::vector<QString> &values)
{
	keys_ = keys;
	values_ = values;
}

//////////////////////////////////////////

Line::Line()
{
	visible_ = true;
	color_ = QColor(Qt::black);
	casingColor_ = QColor(Qt::white);
	casingWidth_ = 0.0;
	width_ = 1.0;
	opacity_ = 1;
	smooth_ = 0;
}

Area::Area()
{
	visible_ = true;
	color_ = QColor(Qt::blue);

	opacity_ = 1.0;

	casingWidth_ = 0;
	casingColor_ = QColor(Qt::black);
}

//////////////////////////////////////////


StyleLayer::StyleLayer(QString dataSource, QString key, StyleLayerType type)
{
	type_ = type;
	key_ = key;
	dataSource_ = dataSource;
}

StyleLayer::StyleLayer(QDomElement layerNode)
{
	key_ = layerNode.attributes().namedItem("k").nodeValue();
	dataSource_ = layerNode.attributes().namedItem("dataSource").nodeValue();

	QString typeStr = layerNode.attributes().namedItem("type").nodeValue();

	QDomNodeList subLayers = layerNode.elementsByTagName("subLayer");
	for (int i = 0; i < subLayers.length(); ++i)
	{
		std::vector<StyleSelector> selectors;

		selectors_.push_back(selectors);
	}

	if (typeStr == "label")
	{
		type_ = ST_LABEL;
	}
	else if (typeStr == "line")
	{
		type_ = ST_LINE;		

		for (int i = 0; i < subLayers.length(); ++i)
		{
			Line line;

			QDomElement lineNode = subLayers.at(i).firstChildElement("line");

			if (lineNode.isNull() == false)
			{
				line.name_ = subLayers.at(i).attributes().namedItem("name").nodeValue();
				line.visible_ = !(subLayers.at(i).attributes().namedItem("visible").nodeValue() == "false");

				line.color_ = lineNode.firstChildElement("color").text();
				line.casingColor_ = lineNode.firstChildElement("casingColor").text();
				line.width_ = lineNode.firstChildElement("width").text().toDouble();
				line.casingWidth_ = lineNode.firstChildElement("casingWidth").text().toDouble();
				line.opacity_ = lineNode.firstChildElement("opacity").text().toDouble();
				line.smooth_ = lineNode.firstChildElement("smooth").text().toDouble();
			}

			lines_.push_back(line);
		}
	}
	else if (typeStr == "point")
	{
		type_ = ST_POINT;
	}
	else if (typeStr == "area")
	{
		type_ = ST_AREA;

		for (int i = 0; i < subLayers.length(); ++i)
		{
			Area area;

			QDomElement lineNode = subLayers.at(i).firstChildElement("area");

			if (lineNode.isNull() == false)
			{
				area.name_ = subLayers.at(i).attributes().namedItem("name").nodeValue();
				area.visible_ = !(subLayers.at(i).attributes().namedItem("visible").nodeValue() == "false");

				area.color_ = lineNode.firstChildElement("color").text();
				area.casingColor_ = lineNode.firstChildElement("casingColor").text();
				area.casingWidth_ = lineNode.firstChildElement("casingWidth").text().toDouble();
				area.opacity_ = lineNode.firstChildElement("opacity").text().toDouble();
			}

			areas_.push_back(area);
		}

	}
	else
	{
		// TODO
	}

	if (key_.isNull())
	{
		// TODO
	}
}

StyleLayer::~StyleLayer()
{
}

QString StyleLayer::key()
{
	return key_;
}

QString StyleLayer::dataSource()
{
	return dataSource_;
}

void StyleLayer::saveXML(QDomDocument &doc, QDomElement &layerElement)
{
	layerElement.setAttribute("k", key());
	layerElement.setAttribute("dataSource", dataSource());

	std::vector<QString> subLayerNamesV = subLayerNames();

	for (size_t i = 0; i < subLayerNamesV.size(); ++i)
	{
		QDomElement subLayerNode = doc.createElement("subLayer");

		switch (layerType())
		{
		case ST_LABEL:
		{
			layerElement.setAttribute("type", "label");
			break;
		}

		case ST_POINT:
		{
			layerElement.setAttribute("type", "point");
			break;
		}

		case ST_LINE:
		{
			layerElement.setAttribute("type", "line");

			QDomElement lineNode = doc.createElement("line");

			Line line = subLayerLine(i);

			subLayerNode.setAttribute("name", line.name_);
			subLayerNode.setAttribute("visible", line.visible_ ? "true" : "false");

			QDomElement colorNode = doc.createElement("color");
			colorNode.appendChild(doc.createTextNode(line.color_.name()));
			lineNode.appendChild(colorNode);

			QDomElement casingColorNode = doc.createElement("casingColor");
			casingColorNode.appendChild(doc.createTextNode(line.casingColor_.name()));
			lineNode.appendChild(casingColorNode);

			QDomElement widthNode = doc.createElement("width");
			widthNode.appendChild(doc.createTextNode(QString::number(line.width_)));
			lineNode.appendChild(widthNode);

			QDomElement casingWidthNode = doc.createElement("casingWidth");
			casingWidthNode.appendChild(doc.createTextNode(QString::number(line.casingWidth_)));
			lineNode.appendChild(casingWidthNode);

			QDomElement opacityNode = doc.createElement("opacity");
			opacityNode.appendChild(doc.createTextNode(QString::number(line.opacity_)));
			lineNode.appendChild(opacityNode);

			QDomElement smoothNode = doc.createElement("smooth");
			smoothNode.appendChild(doc.createTextNode(QString::number(line.smooth_)));
			lineNode.appendChild(smoothNode);

			subLayerNode.appendChild(lineNode);
			break;
		}

		case ST_AREA:
		{
			layerElement.setAttribute("type", "area");

			QDomElement lineNode = doc.createElement("area");

			Area area = subLayerArea(i);

			subLayerNode.setAttribute("name", area.name_);
			subLayerNode.setAttribute("visible", area.visible_ ? "true" : "false");

			QDomElement colorNode = doc.createElement("color");
			colorNode.appendChild(doc.createTextNode(area.color_.name()));
			lineNode.appendChild(colorNode);

			QDomElement opacityNode = doc.createElement("opacity");
			opacityNode.appendChild(doc.createTextNode(QString::number(area.opacity_)));
			lineNode.appendChild(opacityNode);

			QDomElement casingWidthNode = doc.createElement("casingWidth");
			casingWidthNode.appendChild(doc.createTextNode(QString::number(area.casingWidth_)));
			lineNode.appendChild(casingWidthNode);

			QDomElement casingColorNode = doc.createElement("casingColor");
			casingColorNode.appendChild(doc.createTextNode(area.casingColor_.name()));
			lineNode.appendChild(casingColorNode);

			subLayerNode.appendChild(lineNode);
			break;

			break;
		}

		default:
			assert(false);
		}

		layerElement.appendChild(subLayerNode);
	}
}

StyleLayerType StyleLayer::layerType()
{
	return type_;
}

OsmEntityType StyleLayer::dataType()
{
	switch (type_)
	{
	case ST_LABEL: return OET_POINT;
	case ST_POINT: return OET_POINT;
	case ST_LINE: return OET_LINE;
	case ST_AREA: return OET_AREA;
	default:
		assert(false);
	}

	return OET_POINT;
}

void StyleLayer::showAll()
{
	for (auto &line : lines_)
	{
		line.visible_ = true;
	}

}

void StyleLayer::hideAll()
{
	for (auto &line : lines_)
	{
		line.visible_ = false;
	}
}


std::vector<QString> StyleLayer::subLayerNames()
{
	std::vector<QString> ret;

	for (auto &s : selectors_)
	{
		if (s.size() > 0)
		{
			std::vector<QString> keys, values;
			s[0].GetAnds(&keys, &values);
			QString name;
			for (size_t i = 0; i < keys.size(); ++i)
			{
				name += QString("%0=%1").arg(keys[i], values[i]);
				if (i + 1 < keys.size())
					name += ", ";
			}
			ret.push_back(name);
		}
		else
		{
			ret.push_back("All");
		}	
	}

	return ret;
}

std::vector<StyleSelector> StyleLayer::subLayerSelectors(size_t i)
{
	return selectors_[i];
}

void StyleLayer::setSubLayerSelectors(size_t i, const std::vector<StyleSelector> &selections)
{
	selectors_[i] = selections;
}

Area StyleLayer::subLayerArea(size_t i)
{
	return areas_[i];
}

void StyleLayer::setSubLayerArea(size_t i, const Area &area)
{
	if (i < areas_.size())
	{
		areas_[i] = area;
	}
	else if (i == areas_.size())
	{
		areas_.push_back(area);
		selectors_.push_back(std::vector<StyleSelector>());
	}
}


Line StyleLayer::subLayerLine(size_t i)
{
	return lines_[i];
}

void StyleLayer::setSubLayerLine(size_t i, const Line &line)
{
	if (i < lines_.size())
	{
		lines_[i] = line;
	}
	else if (i == lines_.size())
	{
		lines_.push_back(line);
		selectors_.push_back(std::vector<StyleSelector>());
	}
}

