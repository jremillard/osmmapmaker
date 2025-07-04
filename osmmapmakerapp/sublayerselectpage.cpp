#include "sublayerselectpage.h"
#include "ui_sublayerselectpage.h"
#include "selectvalueeditdialog.h"

SubLayerSelectPage::SubLayerSelectPage(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SubLayerSelectPage)
{
    ui->setupUi(this);

    QStringList header = { QString("Key"), QString("Values") };
    ui->terms->setHorizontalHeaderLabels(header);
    db_ = NULL;
}

SubLayerSelectPage::~SubLayerSelectPage()
{
    db_ = NULL;
}

void SubLayerSelectPage::SaveTo(StyleSelector* selector)
{
    selector->clear();

    for (int row = 0; row < ui->terms->rowCount(); ++row) {
        QString key = ui->terms->item(row, 0)->data(Qt::UserRole).toString();

        QStringList values = ui->terms->item(row, 1)->data(Qt::UserRole).toStringList();

        std::vector<QString> valueStd;
        for (auto v : values)
            valueStd.push_back(v);

        selector->insertCondition(row, key, valueStd);
    }
}

void SubLayerSelectPage::Load(RenderDatabase* db, const QString& dataSource, const StyleSelector& selector)
{
    db_ = db;
    dataSource_ = dataSource;

    ui->terms->setRowCount(0);

    size_t count = selector.conditionCount();

    ui->terms->setRowCount(count);

    for (size_t row = 0; row < count; ++row) {
        QString key;
        std::vector<QString> values;
        selector.condition(row, &key, &values);

        AddRow(row, key, values);
    }

    ui->terms->resizeRowsToContents();
}

void SubLayerSelectPage::ValuesToUI(std::vector<QString> values, QString* valuesStr, QStringList* valueQList)
{
    *valuesStr = "";

    for (size_t i = 0; i < values.size(); ++i) {
        valueQList->push_back(values[i]);
        *valuesStr += values[i];
        if (i + 1 < values.size())
            *valuesStr += ", ";
    }
}

void SubLayerSelectPage::on_add_clicked()
{
    SelectValueEditDialog dlg(db_, dataSource_, this);

    if (dlg.exec() == QDialog::Accepted) {
        int row = ui->terms->rowCount();
        ui->terms->setRowCount(row + 1);

        QString key;
        std::vector<QString> values;
        dlg.GetSelections(&key, &values);

        AddRow(row, key, values);

        emit editingFinished();
    }
}

void SubLayerSelectPage::AddRow(int row, const QString& key, std::vector<QString>& values)
{
    ui->terms->setItem(row, 0, new QTableWidgetItem(tr("%0 is").arg(key)));
    ui->terms->item(row, 0)->setData(Qt::UserRole, key);

    QString valuesStr;
    QStringList valueQList;
    ValuesToUI(values, &valuesStr, &valueQList);

    ui->terms->setItem(row, 1, new QTableWidgetItem(valuesStr));
    ui->terms->item(row, 1)->setData(Qt::UserRole, valueQList);

    // no inplace editing.
    ui->terms->item(row, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->terms->item(row, 1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void SubLayerSelectPage::on_terms_cellDoubleClicked(int row, int column)
{
    EditRow(row);
}

void SubLayerSelectPage::on_edit_clicked()
{
    int row = ui->terms->currentRow();
    if (row >= 0)
        EditRow(row);
}

void SubLayerSelectPage::EditRow(int row)
{
    SelectValueEditDialog dlg(db_, dataSource_, this);

    QString key = ui->terms->item(row, 0)->data(Qt::UserRole).toString();

    QStringList values = ui->terms->item(row, 1)->data(Qt::UserRole).toStringList();

    std::vector<QString> valueStd;
    for (auto v : values)
        valueStd.push_back(v);

    dlg.SetSelections(key, valueStd, row != 0);

    if (dlg.exec() == QDialog::Accepted) {
        QString key;
        std::vector<QString> values;
        dlg.GetSelections(&key, &values);

        QString valuesStr;
        QStringList valueQList;
        ValuesToUI(values, &valuesStr, &valueQList);

        ui->terms->item(row, 0)->setText(tr("%0 is").arg(key));
        ui->terms->item(row, 0)->setData(Qt::UserRole, key);

        ui->terms->item(row, 1)->setText(valuesStr);
        ui->terms->item(row, 1)->setData(Qt::UserRole, valueQList);

        ui->terms->resizeRowsToContents();

        emit editingFinished();
    }
}

void SubLayerSelectPage::on_deleteRow_clicked()
{
    int row = ui->terms->currentRow();
    if (row >= 0) {
        ui->terms->removeRow(row);
        emit editingFinished();
    }
}
