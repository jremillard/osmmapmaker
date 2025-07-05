#pragma once

#include <QMainWindow>

#include "project.h"
#include <filesystem>
using std::filesystem::path;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(path projectPath = path(), QWidget* parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();
    void on_action_Project_New_triggered();
    void on_action_Project_Open_triggered();
    void on_action_Project_Copy_triggered();
    void on_action_Project_Save_triggered();

    void openProject(path projectPath);

private:
    Ui::MainWindow* ui;
    Project* project_ = NULL;
};
