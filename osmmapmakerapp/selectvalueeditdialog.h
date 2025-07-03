#pragma once

#include <QDialog>
#include <project.h>
#include <QListWidgetItem>

namespace Ui {
class SelectValueEditDialog;
}

class SelectValueEditDialog : public QDialog {
    Q_OBJECT

public:
    SelectValueEditDialog(SQLite::Database*, const QString& dataSource, QWidget* parent);
    ~SelectValueEditDialog();

    void SetSelections(const QString& key, std::vector<QString>& values, bool allowKeyChange);
    void GetSelections(QString* key, std::vector<QString>* values);

private slots:
    void on_keySearch_textEdited(const QString& text);
    void on_valueSearch_textEdited(const QString& text);
    void on_keySearchList_itemClicked(QListWidgetItem* item);
    void on_valueList_itemSelectionChanged();
    void on_keyHelp_clicked();

private:
    void updateKeyLists();
    void updateValueListFull();
    void updateValueList();

    SQLite::Database* db_;
    QString dataSource_;

    bool surpressSelectChangeSignals_;

    std::vector<QString> selectedValues_;
    std::vector<QString> keys_;
    std::vector<QString> values_;

    Ui::SelectValueEditDialog* ui;
};
