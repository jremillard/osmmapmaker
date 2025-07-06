#pragma once
#include <QDialog>

namespace Ui {
class NewProjectDialog;
}

class NewProjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewProjectDialog(QWidget* parent = nullptr);
    ~NewProjectDialog();

    QString projectPath() const;
    QString templateName() const;

private slots:
    void on_browse_clicked();

private:
    Ui::NewProjectDialog* ui;
};
