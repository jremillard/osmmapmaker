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
	void on_styleNew_clicked();
	void on_styleTree_itemSelectionChanged();
	void on_areaUpdateMap_clicked();
	void on_pointUpdateMap_clicked();

	// map
	void on_mapBackgroundColor_editingFinished();
	void on_mapUpdateMap_clicked();

	// line
	void on_lineVisible_clicked();
	void on_lineColor_editingFinished();
	void on_lineWidth_editingFinished();
	void on_lineDashArray_editingFinished();
	void on_lineCasingWidth_editingFinished();
	void on_lineCasingColor_editingFinished();
	void on_lineSmooth_editingFinished();
	void on_lineOpacity_editingFinished();
	void on_lineUpdateMap_clicked();

private:
	void showEvent(QShowEvent *event);
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);
	void updateTree();
	void freshRender();
	void lineSave();

	int renderImageLeft();

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

