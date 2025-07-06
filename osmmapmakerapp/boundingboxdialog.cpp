#include "boundingboxdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

BoundingBoxDialog::BoundingBoxDialog(QWidget* parent)
    : QDialog(parent)
    , canvas_(new MapCanvas(this))
    , minLat_(new QDoubleSpinBox(this))
    , minLon_(new QDoubleSpinBox(this))
    , maxLat_(new QDoubleSpinBox(this))
    , maxLon_(new QDoubleSpinBox(this))
{
    setWindowTitle(tr("Select Bounding Box"));
    minLat_->setRange(-90.0, 90.0);
    maxLat_->setRange(-90.0, 90.0);
    minLon_->setRange(-180.0, 180.0);
    maxLon_->setRange(-180.0, 180.0);
    minLat_->setDecimals(6);
    maxLat_->setDecimals(6);
    minLon_->setDecimals(6);
    maxLon_->setDecimals(6);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(canvas_);

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Min Lat"), this), 0, 0);
    grid->addWidget(minLat_, 0, 1);
    grid->addWidget(new QLabel(tr("Min Lon"), this), 0, 2);
    grid->addWidget(minLon_, 0, 3);
    grid->addWidget(new QLabel(tr("Max Lat"), this), 1, 0);
    grid->addWidget(maxLat_, 1, 1);
    grid->addWidget(new QLabel(tr("Max Lon"), this), 1, 2);
    grid->addWidget(maxLon_, 1, 3);
    main->addLayout(grid);

    QHBoxLayout* buttons = new QHBoxLayout;
    QPushButton* reset = new QPushButton(tr("Reset"), this);
    QPushButton* ok = new QPushButton(tr("OK"), this);
    QPushButton* cancel = new QPushButton(tr("Cancel"), this);
    buttons->addStretch();
    buttons->addWidget(reset);
    buttons->addWidget(ok);
    buttons->addWidget(cancel);
    main->addLayout(buttons);

    connect(reset, &QPushButton::clicked, this, &BoundingBoxDialog::onReset);
    connect(ok, &QPushButton::clicked, this, &BoundingBoxDialog::onConfirm);
    connect(cancel, &QPushButton::clicked, this, &BoundingBoxDialog::onCancel);
    connect(canvas_, &MapCanvas::boxChanged, this, &BoundingBoxDialog::canvasBoxChanged);

    canvas_->setCenter(0.0, 0.0);
    canvas_->setZoom(2);
}

void BoundingBoxDialog::setInitialBox(const BoundingBoxCoords& box)
{
    currentBox_ = box;
    canvas_->setSelection(box);
    minLat_->setValue(box.minLat);
    minLon_->setValue(box.minLon);
    maxLat_->setValue(box.maxLat);
    maxLon_->setValue(box.maxLon);
}

void BoundingBoxDialog::onReset()
{
    canvas_->setCenter(0.0, 0.0);
    canvas_->setZoom(2);
}

void BoundingBoxDialog::onConfirm()
{
    currentBox_ = canvas_->selection();
    previewImage_ = canvas_->renderSelection();
    accept();
}

void BoundingBoxDialog::onCancel()
{
    reject();
}

void BoundingBoxDialog::canvasBoxChanged(const BoundingBoxCoords& box)
{
    currentBox_ = box;
    minLat_->setValue(box.minLat);
    minLon_->setValue(box.minLon);
    maxLat_->setValue(box.maxLat);
    maxLon_->setValue(box.maxLon);
}
