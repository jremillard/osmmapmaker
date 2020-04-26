#pragma once

#include <QWidget>

#include "project.h"

namespace Ui {
	class DataTab;
}

class DataTab : public QWidget
{
	Q_OBJECT

public:
	DataTab(QWidget *parent = 0);
	~DataTab();

	void setProject(Project *project);

	void UpdateDataSourceList();

private slots:
	void on_deleteData_clicked();
	void on_add_clicked();

private:
	Project *project_ = NULL;

	Ui::DataTab *ui;
};

