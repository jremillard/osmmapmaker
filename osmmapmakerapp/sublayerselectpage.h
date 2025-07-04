#pragma once

#include <QWidget>
#include "stylelayer.h"
#include <project.h>

namespace Ui {
class SubLayerSelectPage;
}

class SubLayerSelectPage : public QWidget {
    Q_OBJECT

public:
    SubLayerSelectPage(QWidget* parent);
    ~SubLayerSelectPage();

    void SaveTo(StyleSelector* sel);
    void Load(RenderDatabase* db, const QString& dataSource, const StyleSelector& sel);

signals:
    void editingFinished();

private slots:
    void on_add_clicked();
    void on_edit_clicked();
    void on_deleteRow_clicked();
    void on_terms_cellDoubleClicked(int row, int column);

private:
    void EditRow(int row);
    void AddRow(int row, const QString& key, std::vector<QString>& values);

    void ValuesToUI(std::vector<QString> values, QString* valuesStr, QStringList* valueQList);

    RenderDatabase* db_;
    QString dataSource_;
    Ui::SubLayerSelectPage* ui;
};
