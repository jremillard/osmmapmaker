#include "projecttemplate.h"
#include <QFile>
#include <stdexcept>

namespace {

static const std::vector<ProjectTemplate::Info> kTemplates = {
    { "Empty", "Empty project with no layers." },
    { "Point", "Single point layer example." },
    { "Line", "Single line layer example." },
    { "Area", "Single area layer example." },
};

static QString templateResource(const QString& id)
{
    QString lower = id.toLower();
    if (lower == "point")
        return ":/resources/templates/template_point.osmmap.xml";
    if (lower == "line")
        return ":/resources/templates/template_line.osmmap.xml";
    if (lower == "area")
        return ":/resources/templates/template_area.osmmap.xml";
    return ":/resources/templates/template_empty.osmmap.xml";
}

} // namespace

const std::vector<ProjectTemplate::Info>& ProjectTemplate::templates()
{
    return kTemplates;
}

QByteArray ProjectTemplate::projectTemplateContents(const QString& id)
{
    QFile in(templateResource(id));
    if (!in.open(QIODevice::ReadOnly))
        throw std::invalid_argument("Unknown template id: " + id.toStdString());
    return in.readAll();
}
