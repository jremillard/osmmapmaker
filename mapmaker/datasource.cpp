
#include "datasource.h"

DataSource::DataSource()
{
    dataName_ = "Primary";
    importDurationS_ = 0;
}

DataSource::DataSource(QDomElement projectNode)
{
    userName_ = projectNode.attributeNode("name").value();

    QDomElement el;

    el = projectNode.firstChildElement("dataSource");
    dataName_ = el.text();

    el = projectNode.firstChildElement("lastUpdateDate");
    if (el.isNull() == false && el.text().isEmpty() == false) {
        lastImport_ = QDateTime::fromString(el.text(), Qt::ISODate);
    }

    importDurationS_ = 0;
    el = projectNode.firstChildElement("importDurationS");
    if (el.isNull() == false && el.text().isEmpty() == false) {
        importDurationS_ = el.text().toInt();
    }
}

DataSource::~DataSource()
{
}

void DataSource::saveXML(QDomDocument& doc, QDomElement& parentElement)
{
    parentElement.setAttribute("name", userName_);

    QDomElement dataSourceElement = doc.createElement("dataSource");
    dataSourceElement.appendChild(doc.createTextNode(dataName_));
    parentElement.appendChild(dataSourceElement);

    QDomElement lastUpdateDateElement = doc.createElement("lastUpdateDate");
    lastUpdateDateElement.appendChild(doc.createTextNode(lastImport_.toString(Qt::ISODate)));
    parentElement.appendChild(lastUpdateDateElement);

    QDomElement lastImportTimeSElement = doc.createElement("importDurationS");
    lastImportTimeSElement.appendChild(doc.createTextNode(QString::number(importDurationS_)));
    parentElement.appendChild(lastImportTimeSElement);
}

QString DataSource::primarySourceName()
{
    return QString("Primary");
}

QString DataSource::userName()
{
    return userName_;
}

void DataSource::setUserName(QString name)
{
    userName_ = name;
}

QString DataSource::dataName()
{
    return dataName_;
}

void DataSource::setDataName(QString name)
{
    dataName_ = name;
}

QDateTime DataSource::importTime()
{
    return lastImport_;
}

void DataSource::setImportTime(QDateTime time)
{
    lastImport_ = time;
}

int DataSource::importDurationS()
{
    return importDurationS_;
}

void DataSource::setImportDurationS(int timeS)
{
    importDurationS_ = timeS;
}

void DataSource::cleanDataSource(RenderDatabase& db)
{
    // clean out the entity, entityKV deleted with a trigger.
    SQLite::Statement removeDataStatement(db, "DELETE FROM entity WHERE source = ?");
    removeDataStatement.bind(1, dataName().toStdString());
    removeDataStatement.exec();

    // clean out the spatial index, can't setup a trigger for it because it is a virtual table.
    SQLite::Statement removeSpatialIndextatement(db, "DELETE FROM entitySpatialIndex WHERE NOT EXISTS (SELECT * FROM entitySpatialIndex,entity WHERE entitySpatialIndex.pkid = entity.id)");
    removeSpatialIndextatement.exec();
}
