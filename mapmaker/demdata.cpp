#include "demdata.h"
#include <QFile>
#include <QTextStream>
#include <stdexcept>
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <cmath>

namespace {

struct Grid {
    int ncols = 0;
    int nrows = 0;
    double xll = 0.0;
    double yll = 0.0;
    double cell = 1.0;
    double nodata = -9999.0;
    std::vector<double> values;
};

static Grid readAsc(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Cannot open DEM file");
    QTextStream ts(&f);
    Grid g;
    for (int i = 0; i < 6; ++i) {
        QString line = ts.readLine();
        QStringList parts = line.trimmed().split(QRegExp("\\s+"));
        if (parts.size() >= 2) {
            QString key = parts.at(0).toLower();
            QString val = parts.at(1);
            if (key == "ncols")
                g.ncols = val.toInt();
            else if (key == "nrows")
                g.nrows = val.toInt();
            else if (key == "xllcorner")
                g.xll = val.toDouble();
            else if (key == "yllcorner")
                g.yll = val.toDouble();
            else if (key == "cellsize")
                g.cell = val.toDouble();
            else if (key == "nodata_value")
                g.nodata = val.toDouble();
        }
    }
    g.values.resize(g.ncols * g.nrows);
    for (int r = 0; r < g.nrows; ++r) {
        QString line = ts.readLine();
        QStringList vals = line.trimmed().split(QRegExp("\\s+"), Qt::SkipEmptyParts);
        for (int c = 0; c < g.ncols && c < vals.size(); ++c) {
            g.values[r * g.ncols + c] = vals.at(c).toDouble();
        }
    }
    return g;
}

static std::string makeWkbLine(double x1, double y1, double x2, double y2)
{
    std::string wkb;
    wkb.resize(1 + 4 + 4 + 2 * 16);
    unsigned char* p = reinterpret_cast<unsigned char*>(wkb.data());
    p[0] = 1; // little endian
    uint32_t* pi = reinterpret_cast<uint32_t*>(p + 1);
    pi[0] = 2; // wkbLineString
    pi[1] = 2; // num points
    double* pd = reinterpret_cast<double*>(p + 1 + 8);
    pd[0] = x1;
    pd[1] = y1;
    pd[2] = x2;
    pd[3] = y2;
    return wkb;
}

} // namespace

DemData::DemData()
    : interval_(10.0)
{
}

DemData::DemData(QDomElement projectNode)
    : DataSource(projectNode)
    , interval_(10.0)
{
    QDomElement el = projectNode.firstChildElement("fileName");
    if (!el.isNull())
        fileName_ = el.text();
    el = projectNode.firstChildElement("interval");
    if (!el.isNull())
        interval_ = el.text().toDouble();
}

DemData::~DemData() { }

void DemData::setFileName(const QString& name)
{
    fileName_ = name;
}

QString DemData::fileName() const
{
    return fileName_;
}

void DemData::setInterval(double v)
{
    interval_ = v;
}

double DemData::interval() const
{
    return interval_;
}

void DemData::importData(RenderDatabase& db)
{
    Grid g = readAsc(fileName_);

    double minV = 0, maxV = 0;
    bool first = true;
    for (double v : g.values) {
        if (v == g.nodata)
            continue;
        if (first) {
            minV = maxV = v;
            first = false;
        } else {
            if (v < minV)
                minV = v;
            if (v > maxV)
                maxV = v;
        }
    }
    if (first)
        return;

    SQLite::Transaction trans(db);
    SQLite::Statement insertEnt(db, "INSERT INTO entity (type, source, geom, linearLengthM, areaM) VALUES (?,?,?,?,?)");
    SQLite::Statement insertKV(db, "INSERT INTO entityKV(id, key, value) VALUES (?,?,?)");
    SQLite::Statement insertIdx(db, "INSERT INTO entitySpatialIndex(pkid,xmin,xmax,ymin,ymax) VALUES (?,?,?,?,?)");

    for (double level = std::ceil(minV / interval_) * interval_; level <= maxV; level += interval_) {
        for (int r = 0; r < g.nrows; ++r) {
            for (int c = 0; c < g.ncols; ++c) {
                double val = g.values[r * g.ncols + c];
                if (val == g.nodata || val < level)
                    continue;
                double x0 = g.xll + c * g.cell;
                double yBottom = g.yll + (g.nrows - r - 1) * g.cell;
                double x1 = x0 + g.cell;
                double yTop = yBottom + g.cell;
                // south boundary
                if (r == g.nrows - 1 || g.values[(r + 1) * g.ncols + c] < level) {
                    std::string wkb = makeWkbLine(x0, yBottom, x1, yBottom);
                    insertEnt.bind(1, static_cast<int>(OET_LINE));
                    insertEnt.bind(2, dataName_.toStdString());
                    insertEnt.bind(3, wkb.c_str(), static_cast<int>(wkb.size()));
                    insertEnt.bind(4, 0.0);
                    insertEnt.bind(5, 0.0);
                    insertEnt.exec();
                    long long id = db.getLastInsertRowid();
                    insertEnt.reset();
                    SQLite::bind(insertKV, static_cast<int64_t>(id), "ele", QString::number(level).toStdString());
                    insertKV.exec();
                    insertKV.reset();
                    SQLite::bind(insertIdx, static_cast<int64_t>(id), x0, x1, yBottom, yBottom);
                    insertIdx.exec();
                    insertIdx.reset();
                }
                // north boundary
                if (r == 0 || g.values[(r - 1) * g.ncols + c] < level) {
                    std::string wkb = makeWkbLine(x0, yTop, x1, yTop);
                    insertEnt.bind(1, static_cast<int>(OET_LINE));
                    insertEnt.bind(2, dataName_.toStdString());
                    insertEnt.bind(3, wkb.c_str(), static_cast<int>(wkb.size()));
                    insertEnt.bind(4, 0.0);
                    insertEnt.bind(5, 0.0);
                    insertEnt.exec();
                    long long id = db.getLastInsertRowid();
                    insertEnt.reset();
                    SQLite::bind(insertKV, static_cast<int64_t>(id), "ele", QString::number(level).toStdString());
                    insertKV.exec();
                    insertKV.reset();
                    SQLite::bind(insertIdx, static_cast<int64_t>(id), x0, x1, yTop, yTop);
                    insertIdx.exec();
                    insertIdx.reset();
                }
                // west boundary
                if (c == 0 || g.values[r * g.ncols + c - 1] < level) {
                    std::string wkb = makeWkbLine(x0, yBottom, x0, yTop);
                    insertEnt.bind(1, static_cast<int>(OET_LINE));
                    insertEnt.bind(2, dataName_.toStdString());
                    insertEnt.bind(3, wkb.c_str(), static_cast<int>(wkb.size()));
                    insertEnt.bind(4, 0.0);
                    insertEnt.bind(5, 0.0);
                    insertEnt.exec();
                    long long id = db.getLastInsertRowid();
                    insertEnt.reset();
                    SQLite::bind(insertKV, static_cast<int64_t>(id), "ele", QString::number(level).toStdString());
                    insertKV.exec();
                    insertKV.reset();
                    SQLite::bind(insertIdx, static_cast<int64_t>(id), x0, x0, yBottom, yTop);
                    insertIdx.exec();
                    insertIdx.reset();
                }
                // east boundary
                if (c == g.ncols - 1 || g.values[r * g.ncols + c + 1] < level) {
                    std::string wkb = makeWkbLine(x1, yBottom, x1, yTop);
                    insertEnt.bind(1, static_cast<int>(OET_LINE));
                    insertEnt.bind(2, dataName_.toStdString());
                    insertEnt.bind(3, wkb.c_str(), static_cast<int>(wkb.size()));
                    insertEnt.bind(4, 0.0);
                    insertEnt.bind(5, 0.0);
                    insertEnt.exec();
                    long long id = db.getLastInsertRowid();
                    insertEnt.reset();
                    SQLite::bind(insertKV, static_cast<int64_t>(id), "ele", QString::number(level).toStdString());
                    insertKV.exec();
                    insertKV.reset();
                    SQLite::bind(insertIdx, static_cast<int64_t>(id), x1, x1, yBottom, yTop);
                    insertIdx.exec();
                    insertIdx.reset();
                }
            }
        }
    }
    trans.commit();
}

void DemData::saveXML(QDomDocument& doc, QDomElement& toElement)
{
    toElement = doc.createElement("elevationSource");
    DataSource::saveXML(doc, toElement);
    QDomElement fn = doc.createElement("fileName");
    fn.appendChild(doc.createTextNode(fileName_));
    toElement.appendChild(fn);
    QDomElement iv = doc.createElement("interval");
    iv.appendChild(doc.createTextNode(QString::number(interval_)));
    toElement.appendChild(iv);
}
