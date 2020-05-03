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

	void createViews();

private:
	void createView(SQLite::Database &db, const QString &viewName, const QString &primaryKey, std::vector<QString> &attributes);

	void createRenderDatabaseIfNotExist();
	void upgradeRenderDatabase();

	std::vector< DataSource*> dataSources_;
	path projectPath_;

};