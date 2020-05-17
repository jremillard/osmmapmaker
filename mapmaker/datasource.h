#pragma once

#include <QString>
#include <QDomElement>
#include <QDateTime>

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

enum OsmEntityType
{
	OET_POINT = 0,
	OET_LINE = 1,
	OET_AREA = 2,
};

class DataSource
{
public:
	DataSource();
	DataSource(QDomElement projectNode);

	virtual ~DataSource();

	virtual QString userName();
	virtual void setUserName(QString name);

	virtual QString dataName();
	virtual void setDataName(QString name);

	virtual QDateTime importTime();
	virtual void setImportTime(QDateTime time);

	virtual int importDurationS();
	virtual void setImportDurationS(int timeS);

	virtual void importData(SQLite::Database &db) = 0;
	virtual void cleanDataSource(SQLite::Database &db);

	static QString primarySourceName();

	virtual void saveXML(QDomDocument &doc, QDomElement &toElement);

protected:

	QString userName_;
	QString dataName_;
	QDateTime lastImport_;
	int importDurationS_;
};


