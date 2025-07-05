#pragma once

#include "datasource.h"

#include <QString>
#include <vector>

/// Imports digital elevation models in ASCII grid format and converts
/// them to contour lines stored in the render database.
class DemData : public DataSource {
public:
    DemData();
    explicit DemData(QDomElement projectNode);
    ~DemData();

    void setFileName(const QString& name);
    QString fileName() const;

    void setInterval(double v);
    double interval() const;

    void importData(RenderDatabase& db) override;
    void saveXML(QDomDocument& doc, QDomElement& toElement) override;

private:
    QString fileName_;
    double interval_;
};
