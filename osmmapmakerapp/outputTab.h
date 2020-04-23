#pragma once

#include <QWidget>

namespace Ui {
	class OutputTab;
}

class OutputTab : public QWidget
{
	Q_OBJECT

public:
	OutputTab(QWidget *parent = 0);
	~OutputTab();

private slots:

private:
	Ui::OutputTab *ui;
};

