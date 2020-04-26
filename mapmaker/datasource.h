#pragma once

#include <QString>
#include <QDomElement>
#include <QDateTime>

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

class DataSource
{
public:
	DataSource();
	DataSource(QDomElement projectNode);

	virtual ~DataSource();

	virtual QString userName();
	virtual QString dataName();
	virtual QDateTime importTime();

	virtual void importData(SQLite::Database &db) = 0;

protected:
	QString userName_;
	QString dataName_;
	QDateTime lastImport_;
	int lastImportTimeS_;
};


