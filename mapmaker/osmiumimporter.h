#pragma once

#include "renderdatabase.h"
#include <QString>
#include <QByteArray>

class OsmiumImporter {
public:
    static void importFile(RenderDatabase& db, const QString& fileName, const QString& dataSource);
    static void importBuffer(RenderDatabase& db, const QByteArray& buffer, const QString& dataSource);
};
