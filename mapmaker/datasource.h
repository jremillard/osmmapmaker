#pragma once

#include <QString>

class dataSource
{
public:
	dataSource();
	virtual ~dataSource();

	virtual QString name();

protected:
	QString name_;

};


