#pragma once

#include <QString>
#include <vector>
#include "datasource.h"
#include "stylelayer.h"

#include <filesystem>

using namespace std::filesystem;

class Project
{
public:
	Project(path filename);
	~Project();

	void save();
	void save(path filename);

	std::vector< DataSource*> dataSources()
	{
		return dataSources_;
	}

	void removeDataSource(DataSource* src);
	void addDataSource(DataSource* src);

	std::vector< StyleLayer*> styleLayers()
	{
		return styleLayers_;
	}

	void removeStyleLayer(StyleLayer* l);
	void addStyleLayer(StyleLayer *l);

	path renderDatabasePath();
	SQLite::Database* renderDatabase();

	path projectPath();

	void createViews();

private:
	void createView(SQLite::Database &db, const QString &viewName, const QString &primaryKey, std::vector<QString> &attributes);

	void createRenderDatabaseIfNotExist();
	void upgradeRenderDatabase();

	SQLite::Database *db_;
	std::vector< DataSource*> dataSources_;
	std::vector< StyleLayer *> styleLayers_;
	path projectPath_;

};