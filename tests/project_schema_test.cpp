#include <catch2/catch_test_macros.hpp>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QFile>
#include <QUrl>

TEST_CASE("Project files validate against schema", "[ProjectSchema]") {
    QXmlSchema schema;
    schema.load(QUrl(QStringLiteral("qrc:/resources/project.xsd")));
    REQUIRE(schema.isValid());

    QStringList files = {
        QStringLiteral(SOURCE_DIR "/projects/groton-sat.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/projects/groton-trail.osmmap.xml")
    };

    for (const QString &fileName : files) {
        QFile f(fileName);
        REQUIRE(f.open(QIODevice::ReadOnly));
        QXmlSchemaValidator validator(schema);
        REQUIRE(validator.validate(&f));
    }
}
