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
	upgradeRenderDatabase();
}

void project::createRenderDatabaseIfNotExist()
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

void project::upgradeRenderDatabase()
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
	}

	transaction.commit();

}


path project::renderDatabasePath()
{
	path dbPath = projectPath_;
	dbPath.replace_extension(".sqlite");
	
	return dbPath;
}


project::~project()
{
	for (auto i : dataSources_)
		delete i;
	dataSources_.clear();
}

