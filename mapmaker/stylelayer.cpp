#include "stylelayer.h"


StyleSelector::StyleSelector()
{
}

StyleSelector::StyleSelector(const QString &key)
{
	keys_.push_back(key);
	values_.push_back(std::vector<QString>() = { QString("*") });
}

StyleSelector::~StyleSelector()
{
}

void StyleSelector::clear()
{
	keys_.clear();
	values_.clear();
}

QString StyleSelector::mapniKExpression()
{
	QString exp;

	if (keys_.size() > 0)
	{
		exp += "(";

		for (size_t i = 0; i < keys_.size(); ++i)
		{
			if (values_[i].size() == 0)
			{
				exp += QString("([%0] = null)").arg(keys_[i]);
			}
			else if (values_[i].size() == 1 && values_[i][0] == "*")
			{
				exp += "1 > 0";
			}
			else
			{
				exp += "(";
				for (size_t v = 0; v < values_[i].size(); ++v)
				{
					// TODO escape '
					exp += QString("([%0]='%1')").arg(keys_[i]).arg(values_[i][v]);

					if (v + 1 < values_[i].size())
						exp += " or ";
				}
				exp += ")";
			}

			if (i + 1 < keys_.size())
				exp += " and ";
		}

		exp += ")";
	}

	return exp;
}

size_t StyleSelector::conditionCount() const
{
	return keys_.size();
}

void StyleSelector::setCondition(size_t i, const QString &key, const std::vector<QString> &values)
{
	keys_[i] = key;
	values_[i] = values;
}

void StyleSelector::condition(size_t i, QString *key, std::vector<QString> *values) const
{
	*key = keys_[i];
	*values = values_[i];
}

void StyleSelector::insertCondition(size_t i, const QString &key, const std::vector<QString> &values)
{
	keys_.insert(keys_.begin() + i, key);
	values_.insert(values_.begin() + i, values);
}

void StyleSelector::deleteCondition(size_t i)
{
	keys_.erase(keys_.begin() + i);
	values_.erase(values_.begin() + i);
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

	fillImageOpacity_ = 1.0;
}

Label::Label()
{
	visible_ = true;
	text_ = "[name]";
	height_ = 10;
	color_ = QColor(Qt::black);
	haloSize_ = 0;
	haloColor_ = QColor(Qt::black);
	lineLaxSpacing_ = 0;
	maxWrapWidth_ = 30;
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
		StyleSelector selector;

		QDomElement selectNode = subLayers.at(i).firstChildElement("selector");

		if (selectNode.isNull() == false)
		{
			QDomNodeList conditionsNodes = selectNode.elementsByTagName("condition");

			for (size_t condIndex = 0; condIndex < conditionsNodes.count(); ++condIndex)
			{
				QString key = conditionsNodes.at(condIndex).attributes().namedItem("key").nodeValue();

				QDomElement condElement = conditionsNodes.at(condIndex).toElement();

				QDomNodeList valueNodes = condElement.elementsByTagName("value");

				std::vector<QString> values;

				for (size_t valueIndex = 0; valueIndex < valueNodes.size(); ++valueIndex)
				{
					QDomElement valNode = valueNodes.at(valueIndex).toElement();

					values.push_back(valNode.text());
				}

				if (values.size() > 0 && key.isEmpty() == false)
					selector.insertCondition(condIndex, key, values);
			}
		}

		// no selector, add one in that is key=*, which does nothing.
		if (selector.conditionCount() == 0)
		{
			selector.insertCondition(0, key_, std::vector<QString>() = { QString("*") });
		}

		selectors_.push_back(selector);
		Label label;
		label.visible_ = false;
		labels_.push_back(label);
	}

	if (typeStr == "line")
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

			QDomElement areaNode = subLayers.at(i).firstChildElement("area");

			if (areaNode.isNull() == false)
			{
				area.name_ = subLayers.at(i).attributes().namedItem("name").nodeValue();
				area.visible_ = !(subLayers.at(i).attributes().namedItem("visible").nodeValue() == "false");

				area.color_ = areaNode.firstChildElement("color").text();
				area.casingColor_ = areaNode.firstChildElement("casingColor").text();
				area.casingWidth_ = areaNode.firstChildElement("casingWidth").text().toDouble();
				area.opacity_ = areaNode.firstChildElement("opacity").text().toDouble();
				area.fillImage_ = areaNode.firstChildElement("fillImage").text();
				area.fillImageOpacity_ = areaNode.firstChildElement("fillImageOpacity").text().toDouble();
			}

			areas_.push_back(area);
		}

	}
	else
	{
		// TODO
	}

	for (int i = 0; i < subLayers.length(); ++i)
	{
		QDomElement labelNode = subLayers.at(i).firstChildElement("label");

		if (labelNode.isNull() == false)
		{
			Label layerLabel = labels_[i];

			layerLabel.visible_ = !(subLayers.at(i).attributes().namedItem("visible").nodeValue() == "false");
			layerLabel.text_ = labelNode.firstChildElement("text").text();
			layerLabel.height_ = labelNode.firstChildElement("height").text().toDouble();
			layerLabel.color_ = labelNode.firstChildElement("color").text();
			layerLabel.haloSize_ = labelNode.firstChildElement("haloSize").text().toDouble();
			layerLabel.haloColor_ = labelNode.firstChildElement("haloColor").text();
			layerLabel.lineLaxSpacing_ = labelNode.firstChildElement("lineLaxSpacing").text().toDouble();
			layerLabel.maxWrapWidth_ = labelNode.firstChildElement("maxWrapWidth").text().toDouble();

			labels_[i] = layerLabel;
		}
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

	for (size_t subLayerIndex = 0; subLayerIndex < subLayerNamesV.size(); ++subLayerIndex)
	{
		QDomElement subLayerNode = doc.createElement("subLayer");

		// selector first
		QDomElement selectorNode = doc.createElement("selector");

		StyleSelector selector = selectors_[subLayerIndex];

		for (size_t condIndex = 0; condIndex < selector.conditionCount(); ++condIndex)
		{
			QDomElement conditionNode = doc.createElement("condition");

			QString key;
			std::vector<QString> values;
			selector.condition(condIndex,&key,&values );

			conditionNode.setAttribute("key",key );

			for (size_t valIndex = 0; valIndex < values.size(); ++valIndex)
			{
				QDomElement valueNode = doc.createElement("value");
				valueNode.appendChild(doc.createTextNode(values[valIndex]));
				conditionNode.appendChild(valueNode);
			}

			selectorNode.appendChild(conditionNode);
		}

		subLayerNode.appendChild(selectorNode);

		// line, area, point

		switch (layerType())
		{
		case ST_POINT:
		{
			layerElement.setAttribute("type", "point");
			break;
		}

		case ST_LINE:
		{
			layerElement.setAttribute("type", "line");

			QDomElement lineNode = doc.createElement("line");

			Line line = subLayerLine(subLayerIndex);

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

			QDomElement areaNode = doc.createElement("area");

			Area area = subLayerArea(subLayerIndex);

			subLayerNode.setAttribute("name", area.name_);
			subLayerNode.setAttribute("visible", area.visible_ ? "true" : "false");

			QDomElement colorNode = doc.createElement("color");
			colorNode.appendChild(doc.createTextNode(area.color_.name()));
			areaNode.appendChild(colorNode);

			QDomElement opacityNode = doc.createElement("opacity");
			opacityNode.appendChild(doc.createTextNode(QString::number(area.opacity_)));
			areaNode.appendChild(opacityNode);

			QDomElement casingWidthNode = doc.createElement("casingWidth");
			casingWidthNode.appendChild(doc.createTextNode(QString::number(area.casingWidth_)));
			areaNode.appendChild(casingWidthNode);

			QDomElement casingColorNode = doc.createElement("casingColor");
			casingColorNode.appendChild(doc.createTextNode(area.casingColor_.name()));
			areaNode.appendChild(casingColorNode);

			QDomElement fillImageNode = doc.createElement("fillImage");
			fillImageNode.appendChild(doc.createTextNode(area.fillImage_));
			areaNode.appendChild(fillImageNode);

			QDomElement fillImageOpacityNode = doc.createElement("fillImageOpacity");
			fillImageOpacityNode.appendChild(doc.createTextNode(QString::number(area.fillImageOpacity_)));
			areaNode.appendChild(fillImageOpacityNode);
			
			subLayerNode.appendChild(areaNode);

			break;
		}

		default:
			assert(false);
		}

		// text labels

		if (labels_[subLayerIndex].visible_)
		{
			Label label = labels_[subLayerIndex];

			QDomElement labelNode = doc.createElement("label");

			QDomElement labelTagNode = doc.createElement("text");
			labelTagNode.appendChild(doc.createTextNode(label.text_));
			labelNode.appendChild(labelTagNode);

			QDomElement heightNode = doc.createElement("height");
			heightNode.appendChild(doc.createTextNode(QString::number(label.height_)));
			labelNode.appendChild(heightNode);

			QDomElement colorNode = doc.createElement("color");
			colorNode.appendChild(doc.createTextNode(label.color_.name()));
			labelNode.appendChild(colorNode);

			QDomElement haloSizeNode = doc.createElement("haloSize");
			haloSizeNode.appendChild(doc.createTextNode(QString::number(label.haloSize_)));
			labelNode.appendChild(haloSizeNode);

			QDomElement haloColorNode = doc.createElement("haloColor");
			haloColorNode.appendChild(doc.createTextNode(label.haloColor_.name()));
			labelNode.appendChild(haloColorNode);

			QDomElement lineLaxSpacingNode = doc.createElement("lineLaxSpacing");
			lineLaxSpacingNode.appendChild(doc.createTextNode(QString::number(label.lineLaxSpacing_)));
			labelNode.appendChild(lineLaxSpacingNode);

			QDomElement maxWrapWidthNode = doc.createElement("maxWrapWidth");
			maxWrapWidthNode.appendChild(doc.createTextNode(QString::number(label.maxWrapWidth_)));
			labelNode.appendChild(maxWrapWidthNode);

			subLayerNode.appendChild(labelNode);
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

	for (auto &area : areas_)
	{
		area.visible_ = true;
	}

}

void StyleLayer::hideAll()
{
	for (auto &line : lines_)
	{
		line.visible_ = false;
	}

	for (auto &area : areas_)
	{
		area.visible_ = false;
	}
}


std::vector<QString> StyleLayer::subLayerNames()
{
	std::vector<QString> ret;

	for (auto &s : selectors_)
	{
		QString key;
		std::vector<QString> values;

		s.condition(0, &key, &values);

		int maxValueDisplay = 35;
		QString valueStr;
		for (int x = 0; x < values.size(); ++x)
		{
			valueStr += values[x];
			if (valueStr.size() > maxValueDisplay)
			{
				valueStr = valueStr.left(maxValueDisplay) + "...";
				break;
			}

			if (x + 1 < values.size())
				valueStr += ", ";
		}

		if (valueStr == "*")
			valueStr = "All";
					
		ret.push_back(valueStr);
	}

	return ret;
}

std::vector<QString> StyleLayer::requiredKeys()
{
	std::vector<QString> keys;

	QRegularExpression re("\\[(\\w+)\\]");

	// labels name tags
	for (int subLayerIndex = 0; subLayerIndex < labels_.size(); ++subLayerIndex)
	{
		if (labels_[subLayerIndex].visible_)
		{
			QRegularExpressionMatchIterator i = re.globalMatch(labels_[subLayerIndex].text_);

			while (i.hasNext()) 
			{
				QRegularExpressionMatch match = i.next();
				keys.push_back(match.captured(1));
			}
		}
	}

	for (int subLayerIndex = 0; subLayerIndex < selectors_.size(); ++subLayerIndex)
	{
		int keyCounditions = selectors_[subLayerIndex].conditionCount();
		for (int keyIndex = 1; keyIndex < keyCounditions; ++keyIndex)
		{
			QString key;
			std::vector<QString> values;
			selectors_[subLayerIndex].condition(keyIndex, &key, &values);
			keys.push_back(key);
		}
	}

	// remove dups
	sort(keys.begin(), keys.end());
	keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

	// primary key should't be listed here, it is always in the table
	keys.erase(std::remove(keys.begin(), keys.end(), key()), keys.end());

	return keys;
}

StyleSelector StyleLayer::subLayerSelectors(size_t i)
{
	return selectors_[i];
}

void StyleLayer::setSubLayerSelectors(size_t i, const StyleSelector &selections)
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
		selectors_.push_back(StyleSelector(key_));
		labels_.push_back(Label());
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
		selectors_.push_back(StyleSelector(key_));
		labels_.push_back(Label());
	}
}

Label StyleLayer::label(size_t i)
{
	return labels_[i];
}

void StyleLayer::setLabel(size_t i, const Label &lb)
{
	labels_[i] = lb;
}



