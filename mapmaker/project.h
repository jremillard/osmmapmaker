#pragma once

#include <QString>
#include <vector>
#include "datasource.h"

class project
{
public:
	project(QString filename);
	~project();

	std::vector< dataSource*> dataSources()
	{
		return dataSources_;
	}


private:

	std::vector< dataSource*> dataSources_;

};