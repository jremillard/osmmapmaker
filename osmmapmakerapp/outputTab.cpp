#include "outputTab.h"
#include "ui_outputTab.h"

#include "batchtileoutput.h"
#include "tileoutput.h"
#include "imageoutput.h"
#include <QProgressDialog>
#include <QApplication>
#include <QInputDialog>

static QIcon tileOutputIcon(QStringLiteral(":/resources/tile_output.svg"));
static QIcon imageOutputIcon(QStringLiteral(":/resources/image_output.svg"));

OutputTab::OutputTab(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::OutputTab)
{
    surpressSelectionChange_ = true;
    ui->setupUi(this);

    ui->tileSize->addItem("256", 256);
    ui->tileSize->addItem("512", 512);
    ui->tileSize->addItem("1024", 1024);

    ui->imageWidth->setValue(256);
    ui->imageHeight->setValue(256);

    surpressSelectionChange_ = false;
    project_ = NULL;
}

OutputTab::~OutputTab()
{
    project_ = NULL;
    delete ui;
}

void OutputTab::setProject(Project* project)
{
    project_ = project;

    ui->outputList->clear();

    for (Output* output : project_->outputs()) {
        QListWidgetItem* item = new QListWidgetItem(output->name());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        if (dynamic_cast<TileOutput*>(output))
            item->setIcon(tileOutputIcon);
        else if (dynamic_cast<ImageOutput*>(output))
            item->setIcon(imageOutputIcon);
        ui->outputList->addItem(item);
    }

    if (ui->outputList->count() > 0)
        ui->outputList->setCurrentRow(0);
    else
        ui->stackedWidget->setCurrentWidget(ui->blankPage);
}

void OutputTab::on_outputList_currentRowChanged(int currentRow)
{
    if (surpressSelectionChange_ == false) {
        if (currentRow < 0) {
            ui->stackedWidget->setCurrentWidget(ui->blankPage);
        } else {
            Output* output = project_->outputs()[currentRow];

            TileOutput* tileOutput = dynamic_cast<TileOutput*>(output);
            ImageOutput* imageOutput = dynamic_cast<ImageOutput*>(output);

            if (tileOutput != NULL) {
                ui->stackedWidget->setCurrentWidget(ui->tilePage);
                ui->generate->setText(tr("Write Tiles"));

                ui->maxZoom->setValue(tileOutput->maxZoom());
                ui->minZoom->setValue(tileOutput->minZoom());
                ui->tileCreate1xRes->setChecked(tileOutput->resolution1x());
                ui->tileCreate2xRes->setChecked(tileOutput->resolution2x());

                if (tileOutput->outputDirectory().size() == 0) {
                    ui->tilePath->setEnabled(false);
                    ui->tileOutputPathUseProjectDir->setChecked(true);
                    saveDefaultPathIntoTilePath();
                } else {
                    ui->tilePath->setEnabled(true);
                    ui->tileOutputPathUseProjectDir->setChecked(false);
                    ui->tilePath->setText(tileOutput->outputDirectory());
                }

                int i = ui->tileSize->findData(tileOutput->tileSizePixels());
                ui->tileSize->setCurrentIndex(i);
            } else if (imageOutput != NULL) {
                ui->stackedWidget->setCurrentWidget(ui->imagePage);
                ui->generate->setText(tr("Write Image"));

                ui->imageWidth->setValue(imageOutput->widthPixels());
                ui->imageHeight->setValue(imageOutput->heightPixels());
                BoundingBox bb = imageOutput->boundingBox();
                ui->imageLongLeft->setValue(bb.left_);
                ui->imageLongRight->setValue(bb.right_);
                ui->imageLatTop->setValue(bb.top_);
                ui->imageLatBottom->setValue(bb.bottom_);
            }
        }
    }
}

void OutputTab::on_outputNew_clicked()
{
    QString newName = QString::fromStdString(project_->projectPath().filename().string());
    newName.replace(".osmmap.xml", "", Qt::CaseInsensitive);

    QStringList opts { tr("Tile Set"), tr("Single Image") };
    bool ok = false;
    QString choice = QInputDialog::getItem(this, tr("New Output"), tr("Output Type"), opts, 0, false, &ok);
    if (!ok)
        return;

    Output* out = nullptr;
    if (choice == opts[1])
        out = new ImageOutput(newName);
    else
        out = new TileOutput(newName);

    project_->addOutput(out);

    QListWidgetItem* item = new QListWidgetItem(out->name());
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    if (dynamic_cast<TileOutput*>(out))
        item->setIcon(tileOutputIcon);
    else
        item->setIcon(imageOutputIcon);

    ui->outputList->addItem(item);
    ui->outputList->setCurrentRow(ui->outputList->count() - 1);
}

void OutputTab::on_outputCopy_clicked()
{
}

void OutputTab::on_outputDelete_clicked()
{
    int currentRow = ui->outputList->currentRow();
    if (currentRow >= 0) {
        surpressSelectionChange_ = true;
        Output* out = project_->outputs()[currentRow];
        project_->removeOutput(out);
        delete out;

        delete ui->outputList->takeItem(currentRow);
        surpressSelectionChange_ = false;

        if (currentRow < ui->outputList->count())
            ui->outputList->setCurrentRow(currentRow);
        else if (ui->outputList->count() > 0)
            ui->outputList->setCurrentRow(ui->outputList->count() - 1);
        else
            on_outputList_currentRowChanged(-1);
    }
}

void OutputTab::on_outputList_itemChanged(QListWidgetItem* item)
{
    int currentRow = ui->outputList->row(item);
    Output* out = project_->outputs()[currentRow];
    out->setName(item->text());
}

void OutputTab::on_maxZoom_editingFinished()
{
    saveTile();
}

void OutputTab::on_minZoom_editingFinished()
{
    saveTile();
}

void OutputTab::on_tileCreate1xRes_clicked()
{
    saveTile();
}

void OutputTab::on_tileCreate2xRes_clicked()
{
    saveTile();
}

void OutputTab::on_tilePath_editingFinished()
{
    saveTile();
}

void OutputTab::on_tileOutputPathUseProjectDir_clicked()
{
    if (ui->tileOutputPathUseProjectDir->isChecked()) {
        saveDefaultPathIntoTilePath();
        ui->tilePath->setEnabled(false);
    } else {
        ui->tilePath->setEnabled(true);
    }
    saveTile();
}

void OutputTab::saveDefaultPathIntoTilePath()
{
    QString outputDirectory = QString::fromStdString(project_->projectPath().string());
    outputDirectory.replace(".xml", "", Qt::CaseInsensitive);
    ui->tilePath->setText(outputDirectory);
    ui->tilePath->setEnabled(false);
}

void OutputTab::on_tileSize_currentIndexChanged(int i)
{
    if (surpressSelectionChange_ == false)
        saveTile();
}

void OutputTab::on_imageWidth_editingFinished()
{
    saveImage();
}

void OutputTab::on_imageHeight_editingFinished()
{
    saveImage();
}

void OutputTab::on_imageLatTop_editingFinished()
{
    saveImage();
}

void OutputTab::on_imageLatBottom_editingFinished()
{
    saveImage();
}

void OutputTab::on_imageLongLeft_editingFinished()
{
    saveImage();
}

void OutputTab::on_imageLongRight_editingFinished()
{
    saveImage();
}

void OutputTab::saveTile()
{

    if (project_ == NULL)
        return;

    int currentRow = ui->outputList->currentRow();

    if (currentRow < 0 || currentRow >= int(project_->outputs().size()))
        return;

    Output* output = project_->outputs()[currentRow];

    TileOutput* tileOutput = dynamic_cast<TileOutput*>(output);

    if (tileOutput != NULL) {
        surpressSelectionChange_ = true;

        tileOutput->setMaxZoom(ui->maxZoom->value());
        tileOutput->setMinZoom(ui->minZoom->value());
        tileOutput->setResolution1x(ui->tileCreate1xRes->isChecked());
        tileOutput->setResolution2x(ui->tileCreate2xRes->isChecked());

        if (ui->tileOutputPathUseProjectDir->isChecked())
            tileOutput->setOutputDirectory(QString());
        else
            tileOutput->setOutputDirectory(ui->tilePath->text());

        int tsize = ui->tileSize->itemData(ui->tileSize->currentIndex()).toInt();
        tileOutput->setTileSizePixels(tsize);

        surpressSelectionChange_ = false;
    }
}

void OutputTab::saveImage()
{
    if (project_ == NULL)
        return;

    int currentRow = ui->outputList->currentRow();

    if (currentRow < 0 || currentRow >= int(project_->outputs().size()))
        return;

    Output* output = project_->outputs()[currentRow];
    ImageOutput* imageOutput = dynamic_cast<ImageOutput*>(output);

    if (imageOutput != NULL) {
        surpressSelectionChange_ = true;

        imageOutput->setWidthPixels(ui->imageWidth->value());
        imageOutput->setHeightPixels(ui->imageHeight->value());
        BoundingBox bb;
        bb.left_ = ui->imageLongLeft->value();
        bb.right_ = ui->imageLongRight->value();
        bb.top_ = ui->imageLatTop->value();
        bb.bottom_ = ui->imageLatBottom->value();
        imageOutput->setBoundingBox(bb);

        surpressSelectionChange_ = false;
    }
}

void OutputTab::on_generate_clicked()
{
    int currentRow = ui->outputList->currentRow();

    Output* output = project_->outputs()[currentRow];

    TileOutput* tileOutput = dynamic_cast<TileOutput*>(output);
    ImageOutput* imageOutput = dynamic_cast<ImageOutput*>(output);

    if (tileOutput != NULL) {
        QProgressDialog dlg("Writing tiles", "Cancel", 0, 0, this);
        dlg.setWindowModality(Qt::WindowModal);

        BatchTileOutput::generateTiles(project_, *tileOutput,
            [&dlg](int done, int total) {
                dlg.setMaximum(total);
                dlg.setValue(done);
                QApplication::processEvents();
                return !dlg.wasCanceled();
            });

        dlg.close();
    } else if (imageOutput != NULL) {
        // image generation not implemented yet
    }
}
