#pragma once

#include <QWidget>

namespace Ui {
	class DataTab;
}

class DataTab : public QWidget
{
	Q_OBJECT

public:
	DataTab(QWidget *parent = 0);
	~DataTab();

private slots:

private:
	Ui::DataTab *ui;
};

