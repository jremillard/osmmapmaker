#pragma once

#include <QDialog>
#include <QColor>
#include <QListWidgetItem>

class Project;

namespace Ui {
class ColorPickerDialog;
}

class ColorPickerDialog : public QDialog {
    Q_OBJECT

public:
    explicit ColorPickerDialog(Project* project, QWidget* parent = nullptr);
    ~ColorPickerDialog();

    QColor selectedColor() const;
    void setCurrentColor(const QColor& color);

    static QColor getColor(Project* project, const QColor& initial, QWidget* parent = nullptr);

private slots:
    void on_usedColors_itemClicked(QListWidgetItem* item);
    void on_usedColors_itemDoubleClicked(QListWidgetItem* item);
    void on_colorWidget_currentColorChanged(const QColor& color);

protected:
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void populateColors();
    void updatePatch(const QColor& color);

    Project* project_;
    QColor color_;
    Ui::ColorPickerDialog* ui;
};
