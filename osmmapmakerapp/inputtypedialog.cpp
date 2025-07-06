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
}

InputTypeDialog::~InputTypeDialog()
{
    delete ui;
}

InputTypeDialog::Choice InputTypeDialog::choice() const
{
    return ui->localRadio->isChecked() ? LocalFile : Overpass;
}

QString InputTypeDialog::fileName() const
{
    return ui->localFilePath->text();
}

void InputTypeDialog::on_localRadio_toggled(bool checked)
{
    ui->browseButton->setEnabled(checked);
    ui->localFilePath->setEnabled(checked);
}

void InputTypeDialog::on_browseButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open OSM File"), QString(), tr("Image Files (*.osm.pbf *.osm)"));
    if (!file.isEmpty())
        ui->localFilePath->setText(QDir::toNativeSeparators(file));
}
