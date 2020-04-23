#pragma once

#include <QWidget>

namespace Ui {
	class StyleTab;
}

class StyleTab : public QWidget
{
	Q_OBJECT

public:
	StyleTab(QWidget *parent);
	~StyleTab();

private slots:

private:
	Ui::StyleTab *ui;
};

