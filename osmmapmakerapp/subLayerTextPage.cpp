#include "subLayerTextPage.h"
#include "ui_subLayerTextPage.h"

#include <QColorDialog>

SubLayerTextPage::SubLayerTextPage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SubLayerTextPage)
{
	ui->setupUi(this);

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

void SubLayerTextPage::on_lineMaxSpacing_editingFinished()
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


void SubLayerTextPage::SaveTo(Label *label)
{
	label->visible_ = ui->hasLabel->isChecked();
	label->text_ = ui->text->text();
	label->height_ = ui->height->value();
	label->color_ = QColor(ui->color->text());
	label->haloSize_ = ui->haloSize->value();
	label->haloColor_ = QColor(ui->haloColor->text());
	label->lineLaxSpacing_ = ui->lineMaxSpacing->value();
	label->maxWrapWidth_ = ui->maxWrapWidth->value();
	label->offsetY_ = ui->offset->value();
}

void SubLayerTextPage::Load(const Label &label)
{
	ui->hasLabel->setChecked(label.visible_);
	ui->text->setText(label.text_);
	ui->height->setValue(label.height_);
	ui->color->setText(label.color_.name());
	ui->haloSize->setValue(label.haloSize_);
	ui->haloColor->setText(label.haloColor_.name());
	ui->lineMaxSpacing->setValue(label.lineLaxSpacing_);
	ui->maxWrapWidth->setValue(label.maxWrapWidth_);
	ui->offset->setValue(label.offsetY_);
}
