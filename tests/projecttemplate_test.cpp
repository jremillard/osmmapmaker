#include <catch2/catch_test_macros.hpp>
#include <QCoreApplication>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QFile>
#include "projecttemplate.h"
#include "project.h"

TEST_CASE("ProjectTemplate templates are valid", "[ProjectTemplate]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    qputenv("QT_BEARER_POLL_TIMEOUT", "-1");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());

    const auto& infos = ProjectTemplate::templates();
    REQUIRE(!infos.empty());

    QXmlSchema schema;
    QFile xsdFile(":/resources/project.xsd");
    REQUIRE(xsdFile.open(QIODevice::ReadOnly));
    schema.load(&xsdFile);
    REQUIRE(schema.isValid());
    QXmlSchemaValidator validator(schema);

    for (const auto& info : infos) {
        std::filesystem::path dir = std::filesystem::temp_directory_path() / ("tmpl_" + info.id.toStdString());
        std::filesystem::create_directories(dir);
        QByteArray bytes = ProjectTemplate::projectTemplateContents(info.id);
        REQUIRE_NOTHROW(Project::createNew("test", dir, bytes));
        std::filesystem::path filePath = dir / "test.osmmap.xml";
        
        // Scope all Qt objects to ensure proper destruction order
        {
            {
                QFile file(QString::fromStdString(filePath.string()));
                REQUIRE(file.open(QIODevice::ReadOnly));
                REQUIRE(validator.validate(&file));
                file.close();
                // file destructor called here, file handle released
            }
            
            {
                Project p(filePath);
                (void)p;
                // p destructor called here, all internal resources released
            }
            
            // All Qt objects destroyed, file handles should be released
        }
        
        std::filesystem::remove_all(dir);
    }

    // Clean up schema resources
    schema = QXmlSchema();
}
