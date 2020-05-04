#pragma once

#include <QWidget>

#include <project.h>

namespace Ui {
	class StyleTab;
}

class StyleTab : public QWidget
{
	Q_OBJECT

public:
	StyleTab(QWidget *parent);
	~StyleTab();

	void setProject(Project *project);

private slots:
	void on_updateMap_clicked();

private:
	void paintEvent(QPaintEvent *event);

	Project *project_;
	QImage renderedImage_;

	Ui::StyleTab *ui;
};

