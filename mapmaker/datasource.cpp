
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

QString DataSource::userName()
{
	return userName_;
}

QString DataSource::dataName()
{
	return dataName_;
}

QDateTime DataSource::importTime()
{
	return lastImport_;
}
