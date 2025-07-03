#pragma once

#include <QWidget>
#include "stylelayer.h"

namespace Ui {
	class SubLayerTextPage;
}

class SubLayerTextPage : public QWidget
{
	Q_OBJECT

public:
	SubLayerTextPage(QWidget *parent);
	~SubLayerTextPage();

	void SaveTo(Label *lb);
	void Load(const Label &lb);

signals:
	void editingFinished();

private slots:
	void on_hasLabel_clicked();
	void on_text_editingFinished();
	void on_height_editingFinished();
	void on_color_editingFinished();
	void on_haloSize_editingFinished();
	void on_haloColor_editingFinished();
	void on_maxWrapWidth_editingFinished();
        void on_offset_editingFinished();
        void on_colorPick_clicked();
        void on_haloColorPick_clicked();
        void on_priority_editingFinished();
        void on_minZoom_editingFinished();
        void on_fontWeight_currentIndexChanged(int i);

private:
	bool suppressUpdates_;
	Ui::SubLayerTextPage *ui;

};





