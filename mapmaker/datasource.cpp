
#include "datasource.h"

DataSource::DataSource()
{
	dataName_ = "Primary";
	lastImportTimeS_ = 0;
}

DataSource::DataSource(QDomElement projectNode)
{
	userName_ = projectNode.attributeNode("name").value();

	QDomElement el;

	el = projectNode.firstChildElement("dataSource");
	dataName_ = el.text();

	el = projectNode.firstChildElement("lastUpdateDate");
	if (el.isNull() == false && el.text().isEmpty() == false)
	{
		lastImport_ = QDateTime::fromString( el.text());
	}

	lastImportTimeS_ = 0;
	el = projectNode.firstChildElement("lastImportTimeS");
	if (el.isNull() == false && el.text().isEmpty() == false)
	{
		lastImportTimeS_ = el.text().toInt();
	}
}

DataSource::~DataSource()
{
}

void DataSource::saveXML(QDomDocument &doc, QDomElement &parentElement)
{
	parentElement.setAttribute("name", userName_);

	QDomElement dataSourceElement = doc.createElement("dataSource");
	dataSourceElement.appendChild(doc.createTextNode(dataName_));
	parentElement.appendChild(dataSourceElement);

	QDomElement lastUpdateDateElement = doc.createElement("lastUpdateDate");
	lastUpdateDateElement.appendChild(doc.createTextNode(lastImport_.toString(Qt::ISODate)));
	parentElement.appendChild(lastUpdateDateElement);

	QDomElement lastImportTimeSElement = doc.createElement("lastImportTimeS");
	lastImportTimeSElement.appendChild(doc.createTextNode(QString::number(lastImportTimeS_)));
	parentElement.appendChild(lastImportTimeSElement);
}


QString DataSource::primarySourceName()
{
	return QString("Primary");
}


QString DataSource::userName()
{
	return userName_;
}

void DataSource::setUserName(QString name)
{
	userName_ = name;
}

QString DataSource::dataName()
{
	return dataName_;
}

void DataSource::setDataName(QString name)
{
	dataName_ = name;
}


QDateTime DataSource::importTime()
{
	return lastImport_;
}

void DataSource::setImportTime(QDateTime time)
{
	lastImport_ = time;
}
