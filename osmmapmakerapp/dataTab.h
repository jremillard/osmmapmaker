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
	void on_OSMFileBrowse_clicked();
	void on_OSMFileImport_clicked();
	void on_OSMFileDelete_clicked();
	void on_addDataSource_clicked();
	void on_dataSources_currentIndexChanged(int index);
	void on_OSMFileName_textChanged(QString text);
	void on_dataSourceUserRename_clicked();
	void on_dataSouceIDEdit_clicked();

private:
	void saveCurrent();

	Project *project_ = NULL;
	int currentIndex_;

	Ui::DataTab *ui;
};

