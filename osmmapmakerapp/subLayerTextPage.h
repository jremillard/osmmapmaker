#pragma once

#include <QWidget>


namespace Ui {
	class SubLayerTextPage;
}


class SubLayerTextPage : public QWidget
{
	Q_OBJECT

public:
	SubLayerTextPage(QWidget *parent);
	~SubLayerTextPage();

private:

	Ui::SubLayerTextPage *ui;


};





