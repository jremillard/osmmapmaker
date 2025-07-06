#pragma once
#include <QDialog>

namespace Ui {
class OutputTypeDialog;
}

class OutputTypeDialog : public QDialog {
    Q_OBJECT
public:
    explicit OutputTypeDialog(QWidget* parent = nullptr);
    ~OutputTypeDialog();

    enum Choice { TileSet,
        SingleImage };
    Choice choice() const;

private:
    Ui::OutputTypeDialog* ui;
};
