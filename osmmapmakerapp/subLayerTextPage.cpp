#include "subLayerTextPage.h"
#include "ui_subLayerTextPage.h"

#include <QColorDialog>

SubLayerTextPage::SubLayerTextPage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SubLayerTextPage)
{
	ui->setupUi(this);

	suppressUpdates_ = true;

	ui->fontWeight->addItem("Extra Light", 200);
	ui->fontWeight->addItem("Normal", 400);
	ui->fontWeight->addItem("Bold", 700);

	suppressUpdates_ = false;
}

SubLayerTextPage::~SubLayerTextPage()
{
}

void SubLayerTextPage::on_hasLabel_clicked()
{
	emit editingFinished();
}

void SubLayerTextPage::on_text_editingFinished()
{
	emit editingFinished();
}

void SubLayerTextPage::on_height_editingFinished()
{
	emit editingFinished();
}

void SubLayerTextPage::on_color_editingFinished()
{
	emit editingFinished();
}

void SubLayerTextPage::on_haloSize_editingFinished()
{
	emit editingFinished();
}

void SubLayerTextPage::on_haloColor_editingFinished()
{
	emit editingFinished();
}

void SubLayerTextPage::on_maxWrapWidth_editingFinished()
{
	emit editingFinished();
}

void SubLayerTextPage::on_offset_editingFinished()
{
	emit editingFinished();
}

void SubLayerTextPage::on_minZoom_editingFinished()
{
	emit editingFinished();
}

void SubLayerTextPage::on_fontWeight_currentIndexChanged(int i)
{
	if (suppressUpdates_ == false)
	{
		emit editingFinished();
	}
}

void SubLayerTextPage::on_colorPick_clicked()
{
	QColor newColor = QColorDialog::getColor(QColor(ui->color->text()), this);

	if (newColor.isValid())
	{
		ui->color->setText(newColor.name());
		emit editingFinished();
	}

}

void SubLayerTextPage::on_haloColorPick_clicked()
{
	QColor newColor = QColorDialog::getColor(QColor(ui->haloColor->text()), this);

	if (newColor.isValid())
	{
		ui->haloColor->setText(newColor.name());
		emit editingFinished();
	}
}

void SubLayerTextPage::on_priority_editingFinished()
{
        emit editingFinished();
}


void SubLayerTextPage::SaveTo(Label *label)
{
	label->visible_ = ui->hasLabel->isChecked();
	label->text_ = ui->text->text();
	label->height_ = ui->height->value();
	label->color_ = QColor(ui->color->text());
	label->haloSize_ = ui->haloSize->value();
        label->haloColor_ = QColor(ui->haloColor->text());
        label->minZoom_ = ui->minZoom->value();
        label->maxWrapWidth_ = ui->maxWrapWidth->value();
        label->offsetY_ = ui->offset->value();
        label->priority_ = ui->priority->value();

	int weight = ui->fontWeight->itemData(ui->fontWeight->currentIndex()).toInt();

	label->fontWeight = weight;
}

void SubLayerTextPage::Load(const Label &label)
{
	ui->hasLabel->setChecked(label.visible_);
	ui->text->setText(label.text_);
	ui->height->setValue(label.height_);
	ui->color->setText(label.color_.name());
	ui->haloSize->setValue(label.haloSize_);
        ui->haloColor->setText(label.haloColor_.name());
        ui->minZoom->setValue(label.minZoom_);
        ui->maxWrapWidth->setValue(label.maxWrapWidth_);
        ui->offset->setValue(label.offsetY_);
        ui->priority->setValue(label.priority_);

	suppressUpdates_ = true;

	int i = ui->fontWeight->findData(label.fontWeight);
	ui->fontWeight->setCurrentIndex(i);

	suppressUpdates_ = false;
}
