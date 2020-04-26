#include "project.h"
#include <QtXml>
#include <exception>

#include "osmdataextractdownload.h"
#include "osmdatadirectdownload.h"
#include "osmdatafile.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>


Project::Project(path fileName)
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

	QDomNodeList dataSources = xmlBOM.elementsByTagName("openStreetMapExtractDownload");
	for (int i = 0; i < dataSources.length(); ++i)
	{
		dataSources_.push_back(new OsmDataExtractDownload(dataSources.at(i)));
	}

	dataSources = xmlBOM.elementsByTagName("openStreetMapDirectDownload");
	for (int i = 0; i < dataSources.length(); ++i)
	{
		dataSources_.push_back(new OsmDataExtractDownload(dataSources.at(i)));
	}

	dataSources = xmlBOM.elementsByTagName("openStreetMapFileSource");
	for (int i = 0; i < dataSources.length(); ++i)
	{
		dataSources_.push_back(new OsmDataFile(dataSources.at(i).toElement()));
	}

	createRenderDatabaseIfNotExist();
	upgradeRenderDatabase();
}

void Project::createRenderDatabaseIfNotExist()
{
	path renderDbPath = renderDatabasePath();
	if (exists(renderDbPath) == false)
	{
		QString nativePath = QString::fromStdWString(renderDbPath.native());

		SQLite::Database db(nativePath.toUtf8().constBegin(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		SQLite::Transaction transaction(db);

		QFile file(":/resources/render-0.sql");
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QByteArray sql = file.readAll();
	
		db.exec(sql.begin());

		transaction.commit();
	}
}

void Project::upgradeRenderDatabase()
{
	path renderDbPath = renderDatabasePath();
	QString nativePath = QString::fromStdWString(renderDbPath.native());

	SQLite::Database db(nativePath.toUtf8().constBegin(), SQLite::OPEN_READWRITE);

	SQLite::Transaction transaction(db);

	const std::string currentSchemaNumber = db.execAndGet("SELECT version FROM version");

	for (int rev = stoi(currentSchemaNumber)+1; true; ++rev)
	{
		QString sqlFileNAme = QString(":/resources/render-%1.sql").arg(rev);

		QFile file(sqlFileNAme);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
			break;
		QByteArray sql = file.readAll();

		db.exec(sql.begin());

		SQLite::Statement query(db, "UPDATE version SET version = ?");
		SQLite::bind(query, rev);
		query.exec();
	}

	transaction.commit();
}


path Project::renderDatabasePath()
{
	path dbPath = projectPath_;
	dbPath.replace_extension(".sqlite");
	
	return dbPath;
}


Project::~Project()
{
	for (auto i : dataSources_)
		delete i;
	dataSources_.clear();
}

