#pragma once

#include <QString>
#include <vector>
#include "datasource.h"

#include <filesystem>

using namespace std::filesystem;


class project
{
public:
	project(path filename);
	~project();

	std::vector< DataSource*> dataSources()
	{
		return dataSources_;
	}


private:
	path renderDatabasePath();

	void createRenderDatabaseIfNotExist();
	void upgradeRenderDatabase();

	std::vector< DataSource*> dataSources_;
	path projectPath_;

};