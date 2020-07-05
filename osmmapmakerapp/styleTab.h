#pragma once

#include <QWidget>

#include <project.h>
#include "subLayerTextPage.h"
#include "sublayerselectpage.h"

class Render;
class RenderQT;

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
	void on_zoom_editingFinished();

	void on_treeNew_clicked();
	void on_treeDelete_clicked();
	void on_treeCopy_clicked();
	void on_treeUp_clicked();
	void on_treeDown_clicked();

	void on_styleTree_itemSelectionChanged();
	void on_pointUpdateMap_clicked();
	void on_updateMap_clicked();

	// map
	void on_mapBackgroundColor_editingFinished();
	void on_mapBackgroundColorPick_clicked();
	void on_mapBackgroundOpacity_valueChanged(double v);

	// layer
	void on_layerShowAll_clicked();
	void on_layerHideAll_clicked();

	// point
	void on_editingFinishedPointLabel();

	// line
	void on_lineVisible_clicked();
	void on_lineColor_editingFinished();
	void on_lineWidth_editingFinished();
	void on_lineDashArray_editingFinished();
	void on_lineCasingWidth_editingFinished();
	void on_lineCasingColor_editingFinished();
	void on_lineSmooth_editingFinished();
	void on_lineOpacity_editingFinished();
	void on_lineCasingColorPick_clicked();
	void on_lineColorPick_clicked();
	void on_editingFinishedLineLabel();
	void on_lineMinZoom_editingFinished();

	// area
	void on_areaVisible_clicked();
	void on_areaColor_editingFinished();
	void on_areaOpacity_editingFinished();
	void on_areaBorderThickness_editingFinished();
	void on_areaBorderColor_editingFinished();
	void on_areaFillImage_editingFinished();
	void on_areaColorPick_clicked();
	void on_areaBorderColorPick_clicked();
	void on_areaFillImageSelect_clicked();
	void on_areaFillImageOpacity_editingFinished();
	void on_editingFinishedAreaLabel();
	void on_areaMinZoom_editingFinished();

private:
	void showEvent(QShowEvent *event);
	void paintEvent(QPaintEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);
	void updateTree();
	void freshRender();
	void saveLine();
	void saveArea();
	void moveTreeItem(int direction);
	void updatePixelResultionFromZoom();

	SubLayerTextPage *lineLabelPage_;
	SubLayerTextPage *areaLabelPage_;
	SubLayerTextPage *pointLabelPage_;
	SubLayerSelectPage *lineSelectPage_;
	SubLayerSelectPage *areaSelectPage_;
	SubLayerSelectPage *pointSelectPage_;

	int renderImageLeft();

	bool mouseTracking_;
	bool supressTreeSelection_;
	QPoint mouseTrackingStart_;
	QPoint mouseTrackingCurrent_;

	Project *project_;
	QImage renderedImage_;
	double centerX_;
	double centerY_;
	double pixelResolution_;

	//Render *render_;
	RenderQT *render_;

	Ui::StyleTab *ui;
};

