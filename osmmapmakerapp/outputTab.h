#pragma once

#include <QWidget>
#include <QListWidgetItem>

#include "renderqt.h"

namespace Ui {
class OutputTab;
}

class OutputTab : public QWidget {
    Q_OBJECT

public:
    OutputTab(QWidget* parent = 0);
    ~OutputTab();

    void setProject(Project* project);

private slots:
    void on_generate_clicked();
    void on_outputNew_clicked();
    void on_outputCopy_clicked();
    void on_outputDelete_clicked();
    void on_outputList_currentRowChanged(int currentRow);
    void on_outputList_itemChanged(QListWidgetItem* item);

    // tile signals
    void on_maxZoom_editingFinished();
    void on_minZoom_editingFinished();
    void on_tileCreate1xRes_clicked();
    void on_tileCreate2xRes_clicked();
    void on_tilePath_editingFinished();
    void on_tileOutputPathUseProjectDir_clicked();
    void on_tileSize_currentIndexChanged(int i);

private:
    void saveTile();
    void saveDefaultPathIntoTilePath();

    bool surpressSelectionChange_;

    Project* project_;

    Ui::OutputTab* ui;
};
