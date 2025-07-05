#pragma once

#include <QString>
#include <vector>
#include "datasource.h"
#include "stylelayer.h"
#include "output.h"
#include "renderdatabase.h"

#include <filesystem>
#include <proj.h>

using namespace std::filesystem;

/// Encapsulates an osmmapmaker project file and its associated
/// render database, style layers and outputs.
class Project {
public:
    Project(path filename);
    ~Project();

    /// Create a new project from a template. Throws std::runtime_error on failure.
    static void createNew(const QString& projectName,
        const std::filesystem::path& directory,
        const QByteArray& templateData);

    void save();
    void save(path filename);

    std::vector<DataSource*> dataSources()
    {
        return dataSources_;
    }

    void removeDataSource(DataSource* src);
    void addDataSource(DataSource* src);

    std::vector<Output*> outputs()
    {
        return outputs_;
    }

    void removeOutput(Output* src);
    void addOutput(Output* src);

    std::vector<StyleLayer*> styleLayers()
    {
        return styleLayers_;
    }

    void removeStyleLayer(StyleLayer* l);
    void addStyleLayer(size_t atIndex, StyleLayer* l);

    double backgroundOpacity();
    void setBackgroundOpacity(double p);

    QColor backgroundColor();
    void setBackgroundColor(QColor c);

    path renderDatabasePath();
    RenderDatabase* renderDatabase();

    path assetDirectory();
    path projectPath();

    void createViews();

    std::string mapSRS();
    std::string dataSRS();

    void convertMapToData(double x, double y, double* lon, double* lat);
    void convertDataToMap(double lat, double lon, double* x, double* y);

    PJ* mapToData_;
    PJ* dataToMap_;

private:
    void createView(RenderDatabase& db, const QString& viewName, const QString& dataSource, OsmEntityType type, const QString& primaryKey, std::vector<QString>& attributes);

    void createRenderDatabaseIfNotExist();

    double backgroundOpacity_;
    QColor backgroundColor_;
    RenderDatabase* db_;
    std::vector<DataSource*> dataSources_;
    std::vector<StyleLayer*> styleLayers_;
    std::vector<Output*> outputs_;
    path projectPath_;

    PJ_CONTEXT* proj_context_;
};