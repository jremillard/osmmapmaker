#pragma once
#include <QDialog>
#include <QImage>
#include "mapcanvas.h"

class QDoubleSpinBox;

class BoundingBoxDialog : public QDialog {
    Q_OBJECT
public:
    explicit BoundingBoxDialog(QWidget* parent = nullptr);

    BoundingBoxCoords selectedBox() const { return currentBox_; }
    QImage selectedImage() const { return previewImage_; }

private slots:
    void onReset();
    void onConfirm();
    void onCancel();
    void canvasBoxChanged(const BoundingBoxCoords& box);

private:
    MapCanvas* canvas_;
    QDoubleSpinBox *minLat_, *minLon_, *maxLat_, *maxLon_;
    QImage previewImage_;
    BoundingBoxCoords currentBox_;
};
