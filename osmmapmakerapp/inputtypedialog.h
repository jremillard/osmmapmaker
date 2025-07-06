#pragma once
#include <QDialog>

namespace Ui {
class InputTypeDialog;
}

class InputTypeDialog : public QDialog {
    Q_OBJECT
public:
    explicit InputTypeDialog(QWidget* parent = nullptr);
    ~InputTypeDialog();

    enum Choice { LocalFile,
        Overpass };
    Choice choice() const;
    QString fileName() const;

private slots:
    void on_localRadio_toggled(bool checked);
    void on_browseButton_clicked();

private:
    Ui::InputTypeDialog* ui;
};
