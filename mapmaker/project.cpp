#include "project.h"
#include <QtXml>
#include <exception>

#include "osmdatadownload.h"

project::project(QString fileName)
{
	QDomDocument xmlBOM;

	// Load xml file as raw data
	QFile f(fileName);

	if (!f.open(QIODevice::ReadOnly))
	{
		auto s = QString("Can't open file %1.").arg(fileName);
		throw std::exception(s.toUtf8());
	}

	// Set data into the QDomDocument before processing
	xmlBOM.setContent(&f);
	f.close();

	//openStreetMapDirectDownload nodes
	QDomNodeList directDownloads = xmlBOM.elementsByTagName("openStreetMapDirectDownload");
	for (int i = 0; i < directDownloads.length(); ++i)
	{
		dataSources_.push_back( new OsmDataDownload(directDownloads.at(i)));
	}



}

project::~project()
{
	for (auto i : dataSources_)
		delete i;
	dataSources_.clear();
}

