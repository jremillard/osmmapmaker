#include <catch2/catch_test_macros.hpp>
#include <QCoreApplication>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QFile>
#include "project.h"
#include "projecttemplate.h"

TEST_CASE("Project templates create valid files", "[Project]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    qputenv("QT_BEARER_POLL_TIMEOUT", "-1");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());

    QStringList templates = { "Empty", "Point", "Line", "Area" };

    QXmlSchema schema;
    QFile xsdFile(":/resources/project.xsd");
    REQUIRE(xsdFile.open(QIODevice::ReadOnly));
    schema.load(&xsdFile);
    REQUIRE(schema.isValid());

    for (const QString& t : templates) {
        std::filesystem::path dir = std::filesystem::temp_directory_path() / ("tmp_" + t.toStdString());
        std::filesystem::create_directories(dir);
        QByteArray bytes = ProjectTemplate::projectTemplateContents(t);
        REQUIRE_NOTHROW(Project::createNew("test", dir, bytes));
        std::filesystem::path tmp = dir / "test.osmmap.xml";
        QFile f(QString::fromStdString(tmp.string()));
        REQUIRE(f.open(QIODevice::ReadOnly));
        QXmlSchemaValidator validator(schema);
        REQUIRE(validator.validate(&f));
        f.close();
        Project proj(tmp);
        (void)proj; // ensure it loads
        std::filesystem::remove_all(dir);
    }

    schema = QXmlSchema();
    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
