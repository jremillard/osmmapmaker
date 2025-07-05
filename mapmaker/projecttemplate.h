#pragma once
#include <QString>
#include <QByteArray>
#include <filesystem>
#include <vector>

/// Provides access to built-in project templates embedded as Qt resources.
class ProjectTemplate {
public:
    struct Info {
        QString id;
        QString description;
    };

    static const std::vector<Info>& templates();
    static QByteArray projectTemplateContents(const QString& id);
};
