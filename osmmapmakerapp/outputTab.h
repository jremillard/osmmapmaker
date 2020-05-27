#pragma once

#include <QWidget>

#include "render.h"

namespace Ui {
	class OutputTab;
}

class OutputTab : public QWidget
{
	Q_OBJECT

public:
	OutputTab(QWidget *parent = 0);
	~OutputTab();

	void setProject(Project *project);


private slots:
	void on_generate_clicked();
 

private:
	std::pair<double, double> fromPixelToLL(std::pair<double, double> px, int zoom);
	std::pair<double, double> fromLLtoPixel(std::pair<double, double> ll, int zoom);

	void RenderTile(Render &render, const path &imagePath, int x, int y, int z);


	Project *project_;

	Ui::OutputTab *ui;
};

