#include <catch2/catch_test_macros.hpp>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QFile>
#include <QUrl>
#include <QCoreApplication>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QEvent>

TEST_CASE("Project files validate against schema", "[ProjectSchema]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());
    Q_INIT_RESOURCE(mapmaker_resources);
    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);
    QXmlSchema schema;
    QFile xsdFile(":/resources/project.xsd");
    REQUIRE(xsdFile.open(QIODevice::ReadOnly));
    schema.load(&xsdFile);
    REQUIRE(schema.isValid());

    QStringList files = {
        QStringLiteral(SOURCE_DIR "/projects/groton-sat.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/projects/groton-trail.osmmap.xml")
    };

    for (const QString& fileName : files) {
        QFile f(fileName);
        REQUIRE(f.open(QIODevice::ReadOnly));
        QXmlSchemaValidator validator(schema);
        REQUIRE(validator.validate(&f));
    }
    schema = QXmlSchema();
    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
