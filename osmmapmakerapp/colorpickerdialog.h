#pragma once

#include <QDialog>
#include <QColor>
#include <QPoint>
#include <QSize>

class Project;

namespace Ui {
class ColorPickerDialog;
}

class ColorPickerDialog : public QDialog {
    Q_OBJECT

public:
    explicit ColorPickerDialog(Project* project, const QString& item,
        QWidget* parent = nullptr);
    ~ColorPickerDialog();

    QColor selectedColor() const;
    void setCurrentColor(const QColor& color);

    static QColor getColor(Project* project, const QColor& initial,
        const QString& item, QWidget* parent = nullptr);

private slots:
    void onColorTableCellClicked(int row, int column);
    void onHueSliderChanged(int v);
    void onSatSliderChanged(int v);
    void onValSliderChanged(int v);
    void onHueEdited();
    void onSatEdited();
    void onValEdited();
    void onHtmlEdited();
    void onStandardPicker();
    void onCssPicker();
    void onDismissHint();
    void onHeaderClicked(int index);

protected:
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void populateColors();
    void updatePatch(const QColor& color);
    int findRow(const QColor& color) const;

    Project* project_;
    QColor color_;
    Ui::ColorPickerDialog* ui;
    QString item_;
    static QPoint lastOffset;
    static QSize lastSize;
    static int lastSortColumn;
    static bool showHint;
};
