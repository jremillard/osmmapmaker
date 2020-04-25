#include "project.h"
#include <QtXml>
#include <exception>

#include "osmdatadownload.h"

#include <SQLiteCpp/SQLiteCpp.h>

project::project(path fileName)
{
	projectPath_ = fileName;
	QDomDocument xmlBOM;

	// Load xml file as raw data
	QString pathStrQ(fileName.string().c_str());

	QFile f(pathStrQ);

	if (!f.open(QIODevice::ReadOnly))
	{
		auto s = QString("Can't open file %1.").arg(pathStrQ);
		throw std::exception(s.toUtf8());
	}

	// Set data into the QDomDocument before processing
	xmlBOM.setContent(&f);
	f.close();

	//openStreetMapDirectDownload nodes
	QDomNodeList directDownloads = xmlBOM.elementsByTagName("openStreetMapDirectDownload");
	for (int i = 0; i < directDownloads.length(); ++i)
	{
		dataSources_.push_back(new OsmDataDownload(directDownloads.at(i)));
	}

	createRenderDatabaseIfNotExist();

}

void project::createRenderDatabaseIfNotExist()
{
	path renderDbPath = renderDatabasePath();
	if (exists(renderDbPath) == false)
	{
		SQLite::Database db(renderDbPath.generic_u8string().c_str(), SQLite::OPEN_CREATE);


	}
}

path project::renderDatabasePath()
{
	path dbPath = projectPath_;
	dbPath.replace_extension(".sqlite");
	
}


project::~project()
{
	for (auto i : dataSources_)
		delete i;
	dataSources_.clear();
}

