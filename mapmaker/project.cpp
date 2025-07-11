#include "project.h"
#include <QtXml>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QResource>
#include <QAbstractMessageHandler>
#include <exception>
#include <stdexcept>

#include "osmoverpasssource.h"
#include "osmdatafilesource.h"
#include "projecttemplate.h"
#include "demdata.h"

#include "tileoutput.h"
#include "imageoutput.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

#include <set>

const char* Project::projectDirectoryExtension()
{
    return ".osmmap";
}

const char* Project::projectFileExtension()
{
    return ".osmmap.xml";
}

class SchemaMessageHandler : public QAbstractMessageHandler {
public:
    SchemaMessageHandler()
        : line_(-1)
    {
    }

    QString statusMessage() const { return statusMessage_; }
    int line() const { return line_; }

protected:
    void handleMessage(QtMsgType,
        const QString& description,
        const QUrl&,
        const QSourceLocation& location) override
    {
        statusMessage_ = description;
        if (!location.isNull())
            line_ = location.line();
    }

private:
    QString statusMessage_;
    int line_;
};

// 'fileName' is the full path to the project XML including
// the '.osmmap.xml' extension.
Project::Project(path fileName)
{
    Q_INIT_RESOURCE(mapmaker_resources);
    proj_context_ = proj_context_create();

    mapToData_ = proj_create_crs_to_crs(proj_context_, mapSRS().c_str(), dataSRS().c_str(), NULL);
    dataToMap_ = proj_create_crs_to_crs(proj_context_, dataSRS().c_str(), mapSRS().c_str(), NULL);

    projectPath_ = fileName;
    QDomDocument xmlBOM;

    // Load xml file as raw data
    QString pathStrQ(fileName.string().c_str());

    QFile f(pathStrQ);

    db_ = NULL;

    backgroundOpacity_ = 1;

    if (!f.open(QIODevice::ReadOnly)) {
        auto s = QString("Can't open file %1.").arg(pathStrQ);
        throw std::runtime_error(s.toStdString());
    }

    // Validate project file against XSD before parsing
    SchemaMessageHandler handler;
    QXmlSchema schema;
    schema.setMessageHandler(&handler);

    QFile xsdFile(":/resources/project.xsd");
    if (!xsdFile.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(std::string("Cannot open resource: ") + xsdFile.fileName().toStdString());
    }
    schema.load(&xsdFile);
    xsdFile.close();

    if (!schema.isValid()) {
        throw std::runtime_error("Internal project schema is invalid");
    }

    QXmlSchemaValidator validator(schema);
    validator.setMessageHandler(&handler);

    bool valid = validator.validate(&f);
    f.close();

    if (!valid) {
        QString msg = QString("%1:%2: %3")
                          .arg(pathStrQ)
                          .arg(handler.line())
                          .arg(handler.statusMessage());
        throw std::runtime_error(msg.toStdString());
    }

    if (!f.open(QIODevice::ReadOnly)) {
        auto s = QString("Can't open file %1.").arg(pathStrQ);
        throw std::runtime_error(s.toStdString());
    }

    backgroundColor_ = QColor(0xE0, 0xE0, 0xE0);

    // Set data into the QDomDocument before processing
    xmlBOM.setContent(&f);
    f.close();

    QDomElement projectNode = xmlBOM.firstChildElement("osmmapmakerproject");

    QDomNodeList toplevelNodes = projectNode.childNodes();

    for (int i = 0; i < toplevelNodes.size(); ++i) {
        QDomElement topNode = toplevelNodes.at(i).toElement();
        QString name = topNode.tagName();

        if (name == "openStreetMapFileSource") {
            dataSources_.push_back(new OsmDataFileSource(topNode));
        } else if (name == "overpassSource") {
            dataSources_.push_back(new OsmOverpassSource(nullptr, topNode));
        } else if (name == "elevationSource") {
            dataSources_.push_back(new DemData(topNode));
        } else if (name == "tileOutput") {
            outputs_.push_back(new TileOutput(topNode));
        } else if (name == "imageOutput") {
            outputs_.push_back(new ImageOutput(topNode));
        } else if (name == "map") {
            backgroundColor_ = topNode.attributes().namedItem("backgroundColor").nodeValue();
            backgroundOpacity_ = topNode.attributes().namedItem("backgroundOpacity").nodeValue().toDouble();

            QDomNodeList layers = topNode.childNodes();

            for (int layerI = 0; layerI < layers.length(); ++layerI) {
                QDomElement layerNode = layers.at(layerI).toElement();

                if (layerNode.tagName() == "layer") {
                    styleLayers_.push_back(new StyleLayer(layerNode));
                }
            }
        }
    }

    createRenderDatabaseIfNotExist();

    path renderDbPath = renderDatabasePath();
    QString nativePath = QString::fromStdString(renderDbPath.string());
    db_ = new RenderDatabase(nativePath);
}

Project::~Project()
{
    for (auto i : dataSources_)
        delete i;
    dataSources_.clear();

    for (auto i : styleLayers_)
        delete i;
    styleLayers_.clear();

    for (auto i : outputs_)
        delete i;
    outputs_.clear();

    delete db_;

    proj_destroy(dataToMap_);
    proj_destroy(mapToData_);

    proj_context_destroy(proj_context_);
    proj_context_ = NULL;
}

void Project::createNew(const QString& projectName,
    const std::filesystem::path& directory,
    const QByteArray& templateData)
{
    std::filesystem::path dirPath = directory;
    if (!std::filesystem::exists(dirPath))
        std::filesystem::create_directories(dirPath);

    std::filesystem::path projectFile = dirPath / (projectName.toStdString() + ".osmmap.xml");

    QFile out(QString::fromStdString(projectFile.string()));
    if (!out.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to create project file");
    if (out.write(templateData) != templateData.size()) {
        out.close();
        throw std::runtime_error("Failed to write project file");
    }
    out.close();

    std::filesystem::path assetDir = projectFile;
    assetDir.replace_extension("");
    if (!std::filesystem::exists(assetDir))
        std::filesystem::create_directories(assetDir);

    QString dbPath = QString::fromStdString((assetDir / "render.sqlite").string());
    RenderDatabase db(dbPath);
}

void Project::createRenderDatabaseIfNotExist()
{
    path renderDbPath = renderDatabasePath();
    if (exists(renderDbPath) == false) {
        QString nativePath = QString::fromStdString(renderDbPath.string());

        RenderDatabase db(nativePath);
    }
}

RenderDatabase* Project::renderDatabase()
{
    return db_;
}

path Project::assetDirectory()
{
    path dbPath = projectPath_;
    dbPath.replace_extension("");
    if (exists(dbPath) == false)
        create_directory(dbPath);
    return dbPath;
}

path Project::renderDatabasePath()
{
    return assetDirectory() /= "render.sqlite";
}

path Project::projectPath()
{
    return projectPath_;
}

double Project::backgroundOpacity()
{
    return backgroundOpacity_;
}

void Project::setBackgroundOpacity(double v)
{
    backgroundOpacity_ = v;
}

QColor Project::backgroundColor()
{
    return backgroundColor_;
}

void Project::setBackgroundColor(QColor c)
{
    backgroundColor_ = c;
}

void Project::save()
{
    saveProjectFile(projectPath_);
}

// Write only the project XML to the specified path. This does not
// copy any referenced files or asset directories.
void Project::saveProjectFile(path fileName)
{
    QDomDocument doc;

    QDomProcessingInstruction xmlProcessingInstruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlProcessingInstruction);

    QDomElement root = doc.createElement("osmmapmakerproject");
    doc.appendChild(root);

    for (auto dataSrc : dataSources_) {
        QDomElement dataNode;
        dataSrc->saveXML(doc, dataNode);
        root.appendChild(dataNode);
    }

    for (auto dataOutput : outputs_) {
        QDomElement dataNode;
        dataOutput->saveXML(doc, dataNode);
        root.appendChild(dataNode);
    }

    QDomElement mapElement = doc.createElement("map");
    mapElement.setAttribute("backgroundColor", backgroundColor_.name());
    mapElement.setAttribute("backgroundOpacity", QString::number(backgroundOpacity_));

    root.appendChild(mapElement);

    for (StyleLayer* layer : styleLayers_) {
        QDomElement layerElement = doc.createElement("layer");
        layer->saveXML(doc, layerElement);
        mapElement.appendChild(layerElement);
    }

    QString pathStrQ(fileName.string().c_str());

    QFile file(pathStrQ);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << doc.toString();
        file.close();
    } else {
        auto s = QString("Can't open file %1.").arg(pathStrQ);
        throw std::runtime_error(s.toStdString());
    }
}

// Duplicate the current project to a new location. The provided
// path may omit the standard project extension. Assets and any
// referenced relative files are copied alongside the XML file.
void Project::saveTo(path fileName)
{
    std::error_code ec;

    std::string fnStr = fileName.filename().string();
    const char* projExt = Project::projectFileExtension();
    if (fnStr.size() < strlen(projExt) || fnStr.compare(fnStr.size() - strlen(projExt), strlen(projExt), projExt) != 0)
        fileName.replace_extension(Project::projectFileExtension());

    path dstDir = fileName;
    dstDir.replace_extension("");

    if (exists(fileName))
        std::filesystem::remove(fileName, ec);
    if (exists(dstDir))
        std::filesystem::remove_all(dstDir, ec);

    saveProjectFile(fileName);

    path srcDir = assetDirectory();
    if (exists(srcDir)) {
        create_directories(dstDir);
        copy_options opts = copy_options::recursive | copy_options::overwrite_existing;
        std::filesystem::copy(srcDir, dstDir, opts, ec);
        if (ec)
            throw std::runtime_error(ec.message());
    }

    path srcProjectDir = projectPath_.parent_path();
    path dstProjectDir = fileName.parent_path();
    for (auto* ds : dataSources_) {
        auto* fileSrc = dynamic_cast<OsmDataFileSource*>(ds);
        if (fileSrc) {
            std::filesystem::path srcFile = fileSrc->localFile().toStdString();
            if (!srcFile.is_absolute()) {
                path absSrc = srcProjectDir / srcFile;
                path absDst = dstProjectDir / srcFile;
                if (exists(absSrc)) {
                    create_directories(absDst.parent_path());
                    std::filesystem::copy_file(absSrc, absDst, copy_options::overwrite_existing, ec);
                    if (ec)
                        throw std::runtime_error(ec.message());
                }
            }
        }
    }
}

void Project::createViews()
{
    RenderDatabase* db = renderDatabase();

    SQLite::Transaction transaction(*db);

    std::set<QString> dataSources;

    for (auto projectLayer : styleLayers()) {
        dataSources.insert(projectLayer->key());
    }

    for (auto& dataSourceI : dataSources) {
        std::map<QString, std::vector<QString>> views;

        for (auto projectLayer : styleLayers()) {
            std::vector<QString> attributes = projectLayer->requiredKeys();

            QString key = projectLayer->key();

            auto vi = views.find(key);
            if (vi != views.end()) {
                vi->second.insert(vi->second.end(), attributes.begin(), attributes.end());
                // remove dups
                sort(vi->second.begin(), vi->second.end());
                vi->second.erase(std::unique(vi->second.begin(), vi->second.end()), vi->second.end());
            } else {
                views[key] = attributes;
            }
        }

        for (auto projectLayer : styleLayers()) {
            QString key = projectLayer->key();
            createView(*db, projectLayer->virtualSQLTableName(), projectLayer->dataSource(), projectLayer->dataType(), key, views[key]);
        }
    }

    transaction.commit();
}

void Project::createView(RenderDatabase& db, const QString& viewName, const QString& dataSource, OsmEntityType type, const QString& primaryKey, std::vector<QString>& attributes)
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
    createViewSql += QString("CREATE VIEW \"%1\" as \n").arg(viewName);
    createViewSql += QString("select\n");
    createViewSql += QString("	entity.id,\n");
    createViewSql += QString("	entity.geom,\n");
    createViewSql += QString("	entity.linearLengthM as __length__,\n");
    createViewSql += QString("	entity.areaM as __area__,\n");
    createViewSql += QString("	\"%1\".value as \"%1\",\n").arg(primaryKey);

    for (QString a : attributes) {
        createViewSql += QString("	\"%1\".value as \"%1\",\n").arg(a);
    }
    createViewSql.chop(2); // don't want the last comma.

    createViewSql += QString("\nfrom entity\n");

    createViewSql += QString("JOIN entityKV as \"%1\" on entity.source == '%2' and entity.type == %3 and entity.id == \"%1\".id and \"%1\".key == '%1'").arg(primaryKey).arg(dataSource).arg(type);

    for (QString a : attributes) {
        createViewSql += QString("	left outer join entityKV as \"%1\" on entity.id = \"%1\".id and \"%1\".key == '%1'\n").arg(a);
    }

    db.exec(createViewSql.toStdString());
}

void Project::removeDataSource(DataSource* src)
{
    dataSources_.erase(find(dataSources_.begin(), dataSources_.end(), src));
    delete src;
}

void Project::addDataSource(DataSource* src)
{
    dataSources_.push_back(src);
}

void Project::removeStyleLayer(StyleLayer* l)
{
    styleLayers_.erase(find(styleLayers_.begin(), styleLayers_.end(), l));
}

void Project::addStyleLayer(size_t addAt, StyleLayer* l)
{
    styleLayers_.insert(styleLayers_.begin() + addAt, l);
}

void Project::removeOutput(Output* output)
{
    outputs_.erase(find(outputs_.begin(), outputs_.end(), output));
}

void Project::addOutput(Output* output)
{
    outputs_.push_back(output);
}

std::string Project::mapSRS()
{
    // EPSG:3857
    const std::string srs_merc = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over";

    return srs_merc;
}

std::string Project::dataSRS()
{
    const std::string dataSRS = "+proj=longlat +datum=WGS84 +no_defs";

    return dataSRS;
}

void Project::convertMapToData(double x, double y, double* lon, double* lat)
{
    PJ_COORD in;
    memset(&in, 0, sizeof(in));
    in.xyzt.x = x;
    in.xyzt.y = y;

    PJ_COORD out = proj_trans(mapToData_, PJ_FWD, in);

    *lon = out.lp.lam;
    *lat = out.lp.phi;
}

void Project::convertDataToMap(double lon, double lat, double* x, double* y)
{
    PJ_COORD in;
    memset(&in, 0, sizeof(in));
    in.lp.lam = lon;
    in.lp.phi = lat;

    PJ_COORD out = proj_trans(dataToMap_, PJ_FWD, in);

    *x = out.xyzt.x;
    *y = out.xyzt.y;
}