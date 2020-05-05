#pragma once

#include <QWidget>

#include <project.h>

class Render;

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
	void on_zoomIn_clicked();
	void on_zoomOut_clicked();

private:
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);

	bool mouseTracking_;
	QPoint mouseTrackingStart_;
	QPoint mouseTrackingCurrent_;

	Project *project_;
	QImage renderedImage_;
	double centerX_;
	double centerY_;
	double pixelResolution_;

	Render *render_;

	Ui::StyleTab *ui;
};

