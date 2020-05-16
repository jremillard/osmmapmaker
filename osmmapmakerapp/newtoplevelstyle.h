#pragma once

#include <QDialog>
#include <QListWidgetItem>

#include <project.h>
#include <vector>

class Render;

namespace Ui {
	class NewStopLeveStyle;
}

class NewStopLeveStyle : public QDialog
{
	Q_OBJECT
public:
	NewStopLeveStyle(Project *project, QWidget *parent);
	~NewStopLeveStyle();

	QString dataSource();
	QString styleKey();
	StyleLayerType styleType();

public slots:

	void on_key_textEdited(const QString &text);
	void on_keyList_itemSelectionChanged();
	void on_keyList_itemDoubleClicked(QListWidgetItem *item);

private:
	std::vector<QString> keys_;
	Ui::NewStopLeveStyle  *ui;
};



