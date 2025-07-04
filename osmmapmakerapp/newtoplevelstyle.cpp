#include "newtoplevelstyle.h"
#include "ui_newtoplevelstyle.h"

#include <QPushButton>

#include <QDesktopServices>
#include <QUrl>

NewStopLeveStyle::NewStopLeveStyle(Project* project, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::NewStopLeveStyle)
{
    ui->setupUi(this);

    RenderDatabase* db = project->renderDatabase();

    for (auto data : project->dataSources()) {
        QString name = data->dataName();
        if (name == data->primarySourceName())
            ui->dataSource->setCurrentIndex(ui->dataSource->count());
        ui->dataSource->addItem(name);
    }

    for (const QString& key : db->allKeysByFrequency()) {
        keys_.push_back(key);
        ui->keyList->addItem(key);
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->styleType->addItem(tr("Area"), StyleLayerType::ST_AREA);
    ui->styleType->addItem(tr("Line"), StyleLayerType::ST_LINE);
    ui->styleType->addItem(tr("Point"), StyleLayerType::ST_POINT);
}

NewStopLeveStyle::~NewStopLeveStyle()
{
}

void NewStopLeveStyle::on_keyHelp_clicked()
{
    QString helpURL = QString("https://wiki.openstreetmap.org/wiki/Key:%0").arg(ui->key->text());
    QDesktopServices::openUrl(QUrl(helpURL));
}

QString NewStopLeveStyle::styleKey()
{
    return ui->key->text();
}

StyleLayerType NewStopLeveStyle::styleType()
{
    return (StyleLayerType)ui->styleType->currentData().toInt();
}

QString NewStopLeveStyle::dataSource()
{
    return ui->dataSource->currentText();
}

void NewStopLeveStyle::on_key_textEdited(const QString& text)
{
    ui->keyList->reset();
    ui->keyList->clear();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    for (QString key : keys_) {
        if (key.indexOf(text) == 0)
            ui->keyList->addItem(key);
    }

    for (QString key : keys_) {
        if (key.indexOf(text) > 0)
            ui->keyList->addItem(key);
    }

    for (QString key : keys_) {
        if (key == text) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            break;
        }
    }
}

void NewStopLeveStyle::on_keyList_itemSelectionChanged()
{
    QListWidgetItem* item = ui->keyList->currentItem();

    if (item) {
        ui->key->setText(item->text());
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void NewStopLeveStyle::on_keyList_itemDoubleClicked(QListWidgetItem* item)
{
    on_keyList_itemSelectionChanged();
    accept();
}
