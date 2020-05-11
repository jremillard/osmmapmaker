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

	db_ = NULL;

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

	path renderDbPath = renderDatabasePath();
	QString nativePath = QString::fromStdWString(renderDbPath.native());
	db_ = new SQLite::Database(nativePath.toUtf8().constBegin(), SQLite::OPEN_READWRITE);
	db_->exec("PRAGMA cache_size = -256000");

	upgradeRenderDatabase();
}

Project::~Project()
{
	for (auto i : dataSources_)
		delete i;
	dataSources_.clear();

	for (auto i : styleLayers_)
		delete i;
	styleLayers_.clear();

	delete db_;
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
	SQLite::Transaction transaction(*db_);

	const std::string currentSchemaNumber = db_->execAndGet("SELECT version FROM version");

	for (int rev = stoi(currentSchemaNumber)+1; true; ++rev)
	{
		QString sqlFileNAme = QString(":/resources/render-%1.sql").arg(rev);

		QFile file(sqlFileNAme);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
			break;
		QByteArray sql = file.readAll();

		db_->exec(sql.begin());

		SQLite::Statement query(*db_, "UPDATE version SET version = ?");
		SQLite::bind(query, rev);
		query.exec();
	}

	transaction.commit();
}

SQLite::Database* Project::renderDatabase()
{
	return db_;
}


path Project::renderDatabasePath()
{
	path dbPath = projectPath_;
	dbPath.replace_extension(".sqlite");
	
	return dbPath;
}


path Project::projectPath()
{
	return projectPath_;
}


void Project::save()
{
	save(projectPath_);
}

void Project::save(path filename)
{
}

void Project::createViews()
{
	SQLite::Database *db = renderDatabase();

	SQLite::Transaction transaction(*db);

	std::vector<QString> attributes{ 
		"name", "surface", "oneway",
		"service","maxspeed","lanes","access",
		"ref","tracktype","bicycle","foot",
		"layer","lit","bridge","footway","width","sidewalk",
		"bus","horse","direction","cycleway","operator"};

	createView(*db, "highway_v", "highway",attributes);

	transaction.commit();
}

void Project::createView(SQLite::Database &db, const QString &viewName, const QString &primaryKey, std::vector<QString> &attributes)
{	
	db.exec(QString("DROP VIEW IF EXISTS %1").arg(viewName).toStdString());

	/*
	CREATE VIEW view_name as
	select
		entity.id,
		highway.value as highway,
		name.value as name,
		access.value as access
	from
		entity
	JOIN entityKV as highway on entity.id == highway.id and highway.key == 'highway'
	left OUTER JOIN entityKV as access on entity.id == access.id and access.key = 'access'
	left OUTER JOIN entityKV as name on entity.id == name.id and name.key = 'name'

	*/

	QString createViewSql;
	createViewSql += QString("CREATE VIEW %1 as \n").arg(viewName);
	createViewSql += QString("select\n");
	createViewSql += QString("	entity.id,\n");
	createViewSql += QString("	entity.geom,\n");
	createViewSql += QString("	%1.value as %1,\n").arg(primaryKey);

	for (QString a : attributes)
	{
		createViewSql += QString("	%1.value as %1,\n").arg(a);
	}
	createViewSql.chop(2); // don't want the last comma.

	createViewSql += QString("\nfrom entity\n");

	createViewSql += QString("JOIN entityKV as %1 on entity.id == %1.id and %1.key == '%1'").arg(primaryKey);

	for (QString a : attributes)
	{
		createViewSql += QString("	left outer join entityKV as %1 on entity.id = %1.id and %1.key == '%1'\n").arg(a);
	}

	db.exec(createViewSql.toStdString());
}

void Project::removeDataSource(DataSource* src)
{
	dataSources_.erase(find(dataSources_.begin(), dataSources_.end(),src));
	delete src;
}

void Project::addDataSource(DataSource* src)
{
	dataSources_.push_back(src);
}

void Project::removeStyleLayer(StyleLayer* l)
{
	styleLayers_.erase(find(styleLayers_.begin(), styleLayers_.end(), l));
	delete l;
}

void Project::addStyleLayer(StyleLayer *l)
{
	styleLayers_.insert(styleLayers_.begin(), l);
}
