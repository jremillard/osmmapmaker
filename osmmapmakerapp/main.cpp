#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	try
	{
		MainWindow w;
		w.showMaximized();

		return a.exec();
	}
	catch (std::exception &e)
	{
		QMessageBox msgBox;
		msgBox.setText(QString("Internal Error.\n\n%1").arg(e.what()));
		msgBox.exec();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
