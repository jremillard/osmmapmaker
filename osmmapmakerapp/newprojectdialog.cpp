#include "newprojectdialog.h"
#include "ui_newprojectdialog.h"
#include <QFileDialog>
#include "projecttemplate.h"

NewProjectDialog::NewProjectDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    for (const auto& info : ProjectTemplate::templates()) {
        ui->templateBox->addItem(info.description, info.id);
    }
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

QString NewProjectDialog::projectPath() const
{
    return ui->projectPath->text();
}

QString NewProjectDialog::templateName() const
{
    return ui->templateBox->currentData().toString();
}

void NewProjectDialog::on_browse_clicked()
{
    QString file = QFileDialog::getSaveFileName(this, tr("New Project"), QString(), tr("Map Project Files (*.osmmap.xml)"));
    if (!file.isEmpty())
        ui->projectPath->setText(file);
}
