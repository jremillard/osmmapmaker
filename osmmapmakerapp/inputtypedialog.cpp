#include "inputtypedialog.h"
#include "ui_inputtypedialog.h"
#include <QFileDialog>
#include <QDir>

InputTypeDialog::InputTypeDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::InputTypeDialog)
{
    ui->setupUi(this);
    ui->localRadio->setChecked(true);
    on_localRadio_toggled(true);
    resize(600, 250);
    ui->localFilePath->setMinimumWidth(500);
}

InputTypeDialog::~InputTypeDialog()
{
    delete ui;
}

InputTypeDialog::Choice InputTypeDialog::choice() const
{
    if (ui->localRadio->isChecked())
        return LocalFile;
    else if (ui->overpassRadio->isChecked())
        return Overpass;
    else
        return DemFile;
}

QString InputTypeDialog::fileName() const
{
    return ui->localFilePath->text();
}

void InputTypeDialog::on_localRadio_toggled(bool checked)
{
    ui->browseButton->setEnabled(checked || ui->demRadio->isChecked());
    ui->localFilePath->setEnabled(checked || ui->demRadio->isChecked());
}

void InputTypeDialog::on_demRadio_toggled(bool checked)
{
    ui->browseButton->setEnabled(checked || ui->localRadio->isChecked());
    ui->localFilePath->setEnabled(checked || ui->localRadio->isChecked());
}

void InputTypeDialog::on_browseButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open OSM File"), QString(), tr("Image Files (*.osm.pbf *.osm)"));
    if (!file.isEmpty())
        ui->localFilePath->setText(QDir::toNativeSeparators(file));
}
