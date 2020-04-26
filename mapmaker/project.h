#pragma once

#include <QString>
#include <vector>
#include "datasource.h"

#include <filesystem>

using namespace std::filesystem;

class Project
{
public:
	Project(path filename);
	~Project();

	std::vector< DataSource*> dataSources()
	{
		return dataSources_;
	}

	path renderDatabasePath();

private:

	void createRenderDatabaseIfNotExist();
	void upgradeRenderDatabase();

	std::vector< DataSource*> dataSources_;
	path projectPath_;

};