#include "stylelayer.h"
#include "textfield.h"

StyleSelector::StyleSelector()
{
}

StyleSelector::StyleSelector(const QString &key)
{
	keys_.push_back(key);
	values_.push_back(std::vector<QString>() = { QString("*") });

	keysStd_.push_back(key.toStdString());
	std::vector<std::string> valuesStd;
	for (QString v : values_.back())
	{
		valuesStd.push_back(v.toStdString());
	}

	valuesStd_.push_back(valuesStd);
}

StyleSelector::~StyleSelector()
{
}

void StyleSelector::clear()
{
	keys_.clear();
	values_.clear();

	keysStd_.clear();
	valuesStd_.clear();
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
				exp += "true";
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

	keysStd_[i] = key.toStdString();
	std::vector<std::string> valuesStd;
	for (QString v : values)
	{
		valuesStd.push_back(v.toStdString());
	}

	valuesStd_[i] = valuesStd;

}

void StyleSelector::condition(size_t i, QString *key, std::vector<QString> *values) const
{
	*key = keys_[i];
	*values = values_[i];
}

void StyleSelector::condition(size_t i, std::string *key, std::vector<std::string> *values) const
{
	*key = keysStd_[i];
	*values = valuesStd_[i];
}

void StyleSelector::insertCondition(size_t i, const QString &key, const std::vector<QString> &values)
{
	keys_.insert(keys_.begin() + i, key);
	values_.insert(values_.begin() + i, values);

	keysStd_.insert(keysStd_.begin() + i, key.toStdString());

	std::vector<std::string> valuesStd;
	for (QString v : values)
	{
		valuesStd.push_back(v.toStdString());
	}

	valuesStd_.insert(valuesStd_.begin() + i, valuesStd);
}

void StyleSelector::deleteCondition(size_t i)
{
	keys_.erase(keys_.begin() + i);
	values_.erase(values_.begin() + i);

	keysStd_.erase(keysStd_.begin() + i);
	valuesStd_.erase(valuesStd_.begin() + i);
}

//////////////////////////////////////////

SubLayer::SubLayer()
{
	visible_ = true;
	color_ = QColor(Qt::black);
	minZoom_ = 0;
}

SubLayer::~SubLayer()
{
}

Point::Point()
{
	width_ = 5;
	image_ = "dot";
	opacity_ = 1;
}

Line::Line()
{
	casingColor_ = QColor(Qt::white);
	casingWidth_ = 0.0;
	width_ = 1.0;
	opacity_ = 1;
	smooth_ = 0;
}

Area::Area()
{
	color_ = QColor(Qt::blue);
	opacity_ = 1.0;
	casingWidth_ = 0;
	casingColor_ = QColor(Qt::black);
	fillImageOpacity_ = 1.0;
}

Label::Label()
{
        text_ = "[name]";
        height_ = 10;
        haloSize_ = 0;
        haloColor_ = QColor(Qt::black);
        maxWrapWidth_ = 30;
        offsetY_ = 0;
        fontWeight = 400; // use ccs font-weight system.
        priority_ = 0;
}

QString Label::mapnikText()
{
	return text_;
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
				line.minZoom_ = subLayers.at(i).attributes().namedItem("minZoom").nodeValue().toInt();
				line.visible_ = !(subLayers.at(i).attributes().namedItem("visible").nodeValue() == "false");

				line.color_ = lineNode.firstChildElement("color").text();
				line.casingColor_ = lineNode.firstChildElement("casingColor").text();
				line.width_ = lineNode.firstChildElement("width").text().toDouble();
				line.casingWidth_ = lineNode.firstChildElement("casingWidth").text().toDouble();
				line.opacity_ = lineNode.firstChildElement("opacity").text().toDouble();
				line.smooth_ = lineNode.firstChildElement("smooth").text().toDouble();

				QString dashArrayStr = lineNode.firstChildElement("dashArray").text();

				if (dashArrayStr.isEmpty() == false)
				{
					QStringList dashArray = dashArrayStr.split(",");
					for (QString &dash : dashArray)
					{
						dash = dash.trimmed();
					}

					line.dashArray_.clear();
					for (size_t i = 0; i + 1 < dashArray.size(); i += 2)
					{
						line.dashArray_.push_back(std::pair<double, double>(dashArray[i].toDouble(), dashArray[i + 1].toDouble()));
					}
				}
			}

			lines_.push_back(line);
		}
	}
	else if (typeStr == "point")
	{
		type_ = ST_POINT;

		for (int i = 0; i < subLayers.length(); ++i)
		{
			Point point;

			QDomElement pointNode = subLayers.at(i).firstChildElement("point");

			if (pointNode.isNull() == false)
			{
				point.name_ = subLayers.at(i).attributes().namedItem("name").nodeValue();
				point.minZoom_ = subLayers.at(i).attributes().namedItem("minZoom").nodeValue().toInt();
				point.visible_ = !(subLayers.at(i).attributes().namedItem("visible").nodeValue() == "false");

				point.color_ = pointNode.firstChildElement("color").text();
				point.width_ = pointNode.firstChildElement("width").text().toDouble();
				point.opacity_ = pointNode.firstChildElement("opacity").text().toDouble();
				point.image_ = pointNode.firstChildElement("image").text();
			}

			points_.push_back(point);
		}
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
				area.minZoom_ = subLayers.at(i).attributes().namedItem("minZoom").nodeValue().toInt();
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

	for (int i = 0; i < subLayers.length(); ++i)
	{
		QDomElement labelNode = subLayers.at(i).firstChildElement("label");

		if (labelNode.isNull() == false)
		{
			Label layerLabel = labels_[i];

			layerLabel.minZoom_ = labelNode.attributes().namedItem("minZoom").nodeValue().toInt();

			layerLabel.visible_ = !(subLayers.at(i).attributes().namedItem("visible").nodeValue() == "false");
			layerLabel.text_ = labelNode.firstChildElement("text").text();
			layerLabel.height_ = labelNode.firstChildElement("height").text().toDouble();
			bool ok = false;;
			int weight = labelNode.firstChildElement("weight").text().toDouble(&ok);
			if (ok)
				layerLabel.fontWeight = weight;
			layerLabel.color_ = labelNode.firstChildElement("color").text();
			layerLabel.haloSize_ = labelNode.firstChildElement("haloSize").text().toDouble();
			layerLabel.haloColor_ = labelNode.firstChildElement("haloColor").text();
			layerLabel.maxWrapWidth_ = labelNode.firstChildElement("maxWrapWidth").text().toDouble();

                        layerLabel.offsetY_ = labelNode.firstChildElement("offsetY").text().toDouble();

                        QDomElement priorityNode = labelNode.firstChildElement("priority");
                        if (!priorityNode.isNull())
                                layerLabel.priority_ = priorityNode.text().toInt();

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

			Point point = subLayerPoint(subLayerIndex);

			subLayerNode.setAttribute("name", point.name_);
			subLayerNode.setAttribute("visible", point.visible_ ? "true" : "false");
			subLayerNode.setAttribute("minZoom", point.minZoom_);

			QDomElement pointNode = doc.createElement("point");

			QDomElement fillImageNode = doc.createElement("image");
			fillImageNode.appendChild(doc.createTextNode(point.image_));
			pointNode.appendChild(fillImageNode);

			QDomElement opacityNode = doc.createElement("opacity");
			opacityNode.appendChild(doc.createTextNode(QString::number(point.opacity_)));
			pointNode.appendChild(opacityNode);

			QDomElement colorNode = doc.createElement("color");
			colorNode.appendChild(doc.createTextNode(point.color_.name()));
			pointNode.appendChild(colorNode);

			QDomElement widthNode = doc.createElement("width");
			widthNode.appendChild(doc.createTextNode(QString::number(point.width_)));
			pointNode.appendChild(widthNode);

			subLayerNode.appendChild(pointNode);

			break;
		}

		case ST_LINE:
		{
			layerElement.setAttribute("type", "line");

			QDomElement lineNode = doc.createElement("line");

			Line line = subLayerLine(subLayerIndex);

			subLayerNode.setAttribute("name", line.name_);
			subLayerNode.setAttribute("visible", line.visible_ ? "true" : "false");
			subLayerNode.setAttribute("minZoom", line.minZoom_);


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

			QDomElement dashArrayNode = doc.createElement("dashArray");
			QString dashArrayStr = "";
			for (auto pair : line.dashArray_)
			{
				if (dashArrayStr.length() > 0)
					dashArrayStr += ",";
				dashArrayStr += QString::number(pair.first) + "," + QString::number(pair.second);
			}

			dashArrayNode.appendChild(doc.createTextNode(dashArrayStr));
			lineNode.appendChild(dashArrayNode);
			
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
			subLayerNode.setAttribute("minZoom", area.minZoom_);

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

			labelNode.setAttribute("minZoom", label.minZoom_);

			QDomElement labelTagNode = doc.createElement("text");
			labelTagNode.appendChild(doc.createTextNode(label.text_));
			labelNode.appendChild(labelTagNode);

			QDomElement heightNode = doc.createElement("height");
			heightNode.appendChild(doc.createTextNode(QString::number(label.height_)));
			labelNode.appendChild(heightNode);

			QDomElement weightNode = doc.createElement("weight");
			weightNode.appendChild(doc.createTextNode(QString::number(label.fontWeight)));
			labelNode.appendChild(weightNode);

			QDomElement colorNode = doc.createElement("color");
			colorNode.appendChild(doc.createTextNode(label.color_.name()));
			labelNode.appendChild(colorNode);

			QDomElement haloSizeNode = doc.createElement("haloSize");
			haloSizeNode.appendChild(doc.createTextNode(QString::number(label.haloSize_)));
			labelNode.appendChild(haloSizeNode);

			QDomElement haloColorNode = doc.createElement("haloColor");
			haloColorNode.appendChild(doc.createTextNode(label.haloColor_.name()));
			labelNode.appendChild(haloColorNode);

			QDomElement maxWrapWidthNode = doc.createElement("maxWrapWidth");
			maxWrapWidthNode.appendChild(doc.createTextNode(QString::number(label.maxWrapWidth_)));
			labelNode.appendChild(maxWrapWidthNode);

                        QDomElement offsetYNode = doc.createElement("offsetY");
                        offsetYNode.appendChild(doc.createTextNode(QString::number(label.offsetY_)));
                        labelNode.appendChild(offsetYNode);

                        QDomElement priorityNode = doc.createElement("priority");
                        priorityNode.appendChild(doc.createTextNode(QString::number(label.priority_)));
                        labelNode.appendChild(priorityNode);

			subLayerNode.appendChild(labelNode);
		}

		layerElement.appendChild(subLayerNode);
	}
}

StyleLayerType StyleLayer::layerType() const
{
	return type_;
}

OsmEntityType StyleLayer::dataType() const
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
	for (auto &point : points_)
	{
		point.visible_ = true;
	}

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
	for (auto &point: points_)
	{
		point.visible_ = false;
	}

	for (auto &line : lines_)
	{
		line.visible_ = false;
	}

	for (auto &area : areas_)
	{
		area.visible_ = false;
	}
}

size_t StyleLayer::subLayerCount() const
{
	return selectors_.size();
}


void StyleLayer::subLayerMove(size_t currentIndex, int direction)
{
	switch (type_)
	{
		case ST_POINT:
			std::iter_swap(points_.begin() + currentIndex, points_.begin() + currentIndex + direction);
			break;
		case ST_LINE:
			std::iter_swap(lines_.begin() + currentIndex, lines_.begin() + currentIndex + direction);
			break;
		case ST_AREA:
			std::iter_swap(areas_.begin() + currentIndex, areas_.begin() + currentIndex + direction);
			break;
	}

	std::iter_swap(labels_.begin() + currentIndex, labels_.begin() + currentIndex + direction);
	std::iter_swap(selectors_.begin() + currentIndex, selectors_.begin() + currentIndex + direction);
}

void StyleLayer::removeSubLayer(size_t currentIndex)
{
	switch (type_)
	{
	case ST_POINT:
		points_.erase(points_.begin() + currentIndex);
		break;
	case ST_LINE:
		lines_.erase(lines_.begin() + currentIndex);
		break;
	case ST_AREA:
		areas_.erase(areas_.begin() + currentIndex );
		break;
	}

	labels_.erase(labels_.begin() + currentIndex);
	selectors_.erase(selectors_.begin() + currentIndex);
}


QString StyleLayer::virtualSQLTableName() const
{
	switch (type_)
	{
	case ST_POINT:
		return key_ + "_" + dataSource_ + "_point_v";;
	case ST_LINE:
		return key_ + "_" + dataSource_ + "_line_v";;
	case ST_AREA:
		return key_ + "_" + dataSource_ + "_area_v";;
	default:
		assert(false);
	}

	return "";
}


std::vector<QString> StyleLayer::subLayerNames() const
{
	std::vector<QString> ret;

	for (auto &s : selectors_)
	{
		QString key;
		std::vector<QString> values;

		s.condition(0, &key, &values);

		int maxValueDisplay = 35;
		QString valueStrPrimary;
		for (int x = 0; x < values.size(); ++x)
		{
			valueStrPrimary += values[x];
			if (x + 1 < values.size())
				valueStrPrimary += ", ";
		}


		QString secondaryKeys;
		for (int condition = 1; condition < s.conditionCount(); ++condition)
		{
			s.condition(condition, &key, &values);
			secondaryKeys += QString("%0=").arg(key);
			if (condition + 1 < s.conditionCount())
				secondaryKeys += ", ";
		}

		QString name;

		if (valueStrPrimary.size() > maxValueDisplay)
		{
			name = valueStrPrimary.left(maxValueDisplay) + "...";
		}
		else
		{
			if (secondaryKeys.isEmpty())
				name = valueStrPrimary;
			else
				name = valueStrPrimary + ", " + secondaryKeys;

			if (name.size() > maxValueDisplay)
				name = name.left(maxValueDisplay) + "...";
		}

		if (name == "*")
			name = "All";
					
		ret.push_back(name);
	}

	return ret;
}

std::vector<QString> StyleLayer::requiredKeys() const
{
	std::vector<QString> keys;

	// labels name tags
	for (int subLayerIndex = 0; subLayerIndex < labels_.size(); ++subLayerIndex)
	{
		if (labels_[subLayerIndex].visible_)
		{
			std::set<QString> keysOut;
			TextFieldProcessor::RequiredKeys(labels_[subLayerIndex].text_, &keysOut);
			for (auto i = keysOut.begin(); i != keysOut.end(); ++i)
			{
				keys.push_back(*i);
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

        // primary key shouldn't be listed here; it is always in the table
	keys.erase(std::remove(keys.begin(), keys.end(), key_), keys.end());

	return keys;
}

const StyleSelector& StyleLayer::subLayerSelectors(size_t i)
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

Point StyleLayer::subLayerPoint(size_t i)
{
	return points_[i];
}

void StyleLayer::setSubLayerPoint(size_t i, const Point &point)
{
	if (i < points_.size())
	{
		points_[i] = point;
	}
	else if (i == points_.size())
	{
		points_.push_back(point);
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

QString StyleLayer::renderSQLSelect(bool sortBySize) const
{
	std::vector<QString> attributes = requiredKeys();


	/*
	CREATE VIEW view_name as
	select
		entity.id,
		highway.value as highway,
		name.value as name,
		access.value as access
	from
		entity
	JOIN entityKV as highway on entity.id == highway.id and highway.key == 'highway'
	left OUTER JOIN entityKV as access on entity.id == access.id and access.key = 'access'
	left OUTER JOIN entityKV as name on entity.id == name.id and name.key = 'name'

	*/

	QString pixelWidthInMeters = "(!pixel_width!*6356.0 * 1000.0 * 2.0 * 3.1415926535897932384626433832795 / 360)";
	QString pixelHeightInMeters = "(!pixel_height!*6356.0 * 1000.0 * 2.0 * 3.1415926535897932384626433832795 / 360)";

	QString createViewSql;
	createViewSql += QString("select\n");
	createViewSql += QString("	entity.id,\n");
	createViewSql += QString("	entity.geom,\n");
	//createViewSql += QString("  %1 as __pixelWidth__,\n").arg(pixelWidthInMeters);
	//createViewSql += QString("  %1 as __pixelHeight__,\n").arg(pixelWidthInMeters);
	//createViewSql += QString("	round(entity.linearLengthM/%1) as __lengthPixels__,\n").arg(pixelWidthInMeters);
	//createViewSql += QString("	2*entity.areaM/(%1*%2*3.1415926535897932384626433832795) as __areaDiameterPixels__,\n").arg(pixelWidthInMeters, pixelHeightInMeters);
	createViewSql += QString("	\"%1\".value as \"%1\",\n").arg(key_);

	for (QString a : attributes)
	{
		createViewSql += QString("	\"%1\".value as \"%1\",\n").arg(a);
	}
	createViewSql.chop(2); // don't want the last comma.

	createViewSql += QString("\nfrom entity\n");

	createViewSql += QString("JOIN entityKV as \"%1\" on entity.source == '%2' and entity.type == %3 and entity.id == \"%1\".id and \"%1\".key == '%1'").arg(key_).arg(dataSource_).arg(dataType());

	for (QString a : attributes)
	{
		createViewSql += QString("	left outer join entityKV as \"%1\" on entity.id = \"%1\".id and \"%1\".key == '%1'\n").arg(a);
	}

	if (sortBySize)
	{
		//createViewSql += "ORDER BY __areaDiameterPixels__, __lengthPixels__ DESC\n";
	}

	//createViewSql += "WHERE __area__ > !pixel_width! * !pixel_height!\n";

	//return "( " + createViewSql + ")";
	return createViewSql;
}

