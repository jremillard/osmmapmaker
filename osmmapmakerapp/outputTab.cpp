#include "outputTab.h"
#include "ui_outputTab.h"

#include "batchtileoutput.h"

OutputTab::OutputTab(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::OutputTab)
{
    surpressSelectionChange_ = true;
    ui->setupUi(this);

    ui->tileSize->addItem("256", 256);
    ui->tileSize->addItem("512", 512);
    ui->tileSize->addItem("1024", 1024);

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

            if (tileOutput != NULL) {
                ui->stackedWidget->setCurrentWidget(ui->tilePage);

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
            }
        }
    }
}

void OutputTab::on_outputNew_clicked()
{
    QString newName = QString::fromStdString(project_->projectPath().filename().string());
    newName.replace(".osmmap.xml", "", Qt::CaseInsensitive);
    TileOutput* to = new TileOutput(newName);

    project_->addOutput(to);

    QListWidgetItem* item = new QListWidgetItem(to->name());
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

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

void OutputTab::on_generate_clicked()
{
    int currentRow = ui->outputList->currentRow();

    Output* output = project_->outputs()[currentRow];

    TileOutput* tileOutput = dynamic_cast<TileOutput*>(output);

    if (tileOutput != NULL) {
        BatchTileOutput::generateTiles(project_, *tileOutput);
    }
}
