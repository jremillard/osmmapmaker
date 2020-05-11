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
	color_ = QColor(Qt::black);
	casingColor_ = QColor(Qt::white);
	casingWidth_ = 0.0;
	width_ = 1.0;
	opacity_ = 1;
	smooth_ = 2;
}

//////////////////////////////////////////


StyleLayer::StyleLayer(QString key, StyleLayerType type)
{
	type_ = type;
	key_ = key;
}

StyleLayer::StyleLayer(QDomElement layerNode)
{
}

StyleLayer::~StyleLayer()
{
}

QString StyleLayer::key()
{
	return key_;
}

void StyleLayer::save(QDomElement layerNode)
{

}

StyleLayerType StyleLayer::layerType()
{
	return type_;
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

