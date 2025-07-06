#include "outputtypedialog.h"
#include "ui_outputtypedialog.h"

OutputTypeDialog::OutputTypeDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::OutputTypeDialog)
{
    ui->setupUi(this);
    ui->tileRadio->setChecked(true);
}

OutputTypeDialog::~OutputTypeDialog()
{
    delete ui;
}

OutputTypeDialog::Choice OutputTypeDialog::choice() const
{
    return ui->tileRadio->isChecked() ? TileSet : SingleImage;
}
