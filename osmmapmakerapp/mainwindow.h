#pragma once

#include <QMainWindow>

#include "project.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();
	void openProject(path projectPath);

private:
    Ui::MainWindow *ui;
	Project *project_ = NULL;

};

