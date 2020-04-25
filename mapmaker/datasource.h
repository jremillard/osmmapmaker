#pragma once

#include <QString>

class DataSource
{
public:
	DataSource();
	virtual ~DataSource();

	virtual QString userName();
	virtual QString dataName();

	virtual void import() = 0;

protected:
	QString userName_;
	QString dataName_;

};


