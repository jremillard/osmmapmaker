#include "outputTab.h"
#include "ui_outputTab.h"

#include <filesystem>
#include <algorithm>
#include <math.h>
#include <thread>
#include <algorithm>

OutputTab::OutputTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OutputTab)
{
	ui->setupUi(this);

	ui->tileSize->addItem("256", 256);
	ui->tileSize->addItem("512", 512);
	ui->tileSize->addItem("1024", 1024);

	surpressSelectionChange_ = false;
	project_ = NULL;
}

OutputTab::~OutputTab()
{
	project_ = NULL;
	delete ui;
}

void OutputTab::setProject(Project *project)
{
	project_ = project;

	ui->outputList->clear();

	for (Output *output : project_->outputs())
	{
		QListWidgetItem *item = new QListWidgetItem(output->name());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
		ui->outputList->addItem(item);
	}

	if (ui->outputList->count() > 0)
		ui->outputList->setCurrentRow(0);
	else
		ui->stackedWidget->setCurrentWidget(ui->blankPage);
}

void OutputTab::on_outputList_currentRowChanged(int currentRow)
{
	if (surpressSelectionChange_ == false)
	{
		if (currentRow < 0)
		{
			ui->stackedWidget->setCurrentWidget(ui->blankPage);
		}
		else
		{
			Output *output = project_->outputs()[currentRow];

			TileOutput *tileOutput = dynamic_cast<TileOutput*>(output);

			if (tileOutput != NULL)
			{
				ui->stackedWidget->setCurrentWidget(ui->tilePage);

				ui->maxZoom->setValue( tileOutput->maxZoom());
				ui->minZoom->setValue(tileOutput->minZoom());
				ui->tileCreate1xRes->setChecked(tileOutput->resolution1x());
				ui->tileCreate2xRes->setChecked(tileOutput->resolution2x());

				if (tileOutput->outputDirectory().size() == 0)
				{
					ui->tilePath->setEnabled(false);
					ui->tileOutputPathUseProjectDir->setChecked(true);
					saveDefaultPathIntoTilePath();
				}
				else
				{
					ui->tilePath->setEnabled(true);
					ui->tileOutputPathUseProjectDir->setChecked(false);
					ui->tilePath->setText(tileOutput->outputDirectory());
				}

				int i = ui->tileSize->findData(tileOutput->tileSizePixels());
				ui->tileSize->setCurrentIndex(i);

			}
		}
	}
}

void OutputTab::on_outputNew_clicked()
{
	QString newName = QString::fromStdString(project_->projectPath().filename().string());
	newName.replace(".osmmap.xml", "", Qt::CaseInsensitive);
	TileOutput *to = new TileOutput(newName);

	project_->addOutput(to);

	QListWidgetItem *item = new QListWidgetItem(to->name());
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

	ui->outputList->addItem(item);
	ui->outputList->setCurrentRow(ui->outputList->count() - 1);
}

void OutputTab::on_outputCopy_clicked()
{
}

void OutputTab::on_outputDelete_clicked()
{
	int currentRow = ui->outputList->currentRow();
	if (currentRow >= 0)
	{
		surpressSelectionChange_ = true;
		Output *out = project_->outputs()[currentRow];
		project_->removeOutput(out);
		delete out;

		delete ui->outputList->takeItem(currentRow);
		surpressSelectionChange_ = false;

		if (currentRow < ui->outputList->count())
			ui->outputList->setCurrentRow(currentRow);
		else if (ui->outputList->count() > 0)
			ui->outputList->setCurrentRow(ui->outputList->count() - 1);
		else
			on_outputList_currentRowChanged(-1);
	}
}

void OutputTab::on_outputList_itemChanged(QListWidgetItem *item)
{
	int currentRow = ui->outputList->row(item);
	Output *out = project_->outputs()[currentRow];
	out->setName(item->text());
}

void OutputTab::on_maxZoom_editingFinished()
{
	saveTile();
}

void OutputTab::on_minZoom_editingFinished()
{
	saveTile();
}

void OutputTab::on_tileCreate1xRes_clicked()
{
	saveTile();
}

void OutputTab::on_tileCreate2xRes_clicked()
{
	saveTile();
}

void OutputTab::on_tilePath_editingFinished()
{
	saveTile();
}

void OutputTab::on_tileOutputPathUseProjectDir_clicked()
{
	if (ui->tileOutputPathUseProjectDir->isChecked())
	{
		saveDefaultPathIntoTilePath();
		ui->tilePath->setEnabled(false);
	}
	else
	{
		ui->tilePath->setEnabled(true);
	}
	saveTile();
}

void OutputTab::saveDefaultPathIntoTilePath()
{
	QString outputDirectory = QString::fromStdString(project_->projectPath().string());
	outputDirectory.replace(".xml", "", Qt::CaseInsensitive);
	ui->tilePath->setText(outputDirectory);
	ui->tilePath->setEnabled(false);
}

void OutputTab::on_tileSize_currentIndexChanged(int i)
{
	if (surpressSelectionChange_ == false)
		saveTile();
}

void OutputTab::saveTile()
{
	int currentRow = ui->outputList->currentRow();

	Output *output = project_->outputs()[currentRow];

	TileOutput *tileOutput = dynamic_cast<TileOutput*>(output);

	if (tileOutput != NULL)
	{
		surpressSelectionChange_ = true;

		tileOutput->setMaxZoom(ui->maxZoom->value());
		tileOutput->setMinZoom(ui->minZoom->value());
		tileOutput->setResolution1x(ui->tileCreate1xRes->isChecked());
		tileOutput->setResolution2x(ui->tileCreate2xRes->isChecked());

		if (ui->tileOutputPathUseProjectDir->isChecked())
			tileOutput->setOutputDirectory(QString());
		else
			tileOutput->setOutputDirectory(ui->tilePath->text());

		int tsize = ui->tileSize->itemData(ui->tileSize->currentIndex()).toInt();
		tileOutput->setTileSizePixels(tsize);

		surpressSelectionChange_ = false;
	}

}

void OutputTab::on_generate_clicked()
{
	int currentRow = ui->outputList->currentRow();

	Output *output = project_->outputs()[currentRow];

	TileOutput *tileOutput = dynamic_cast<TileOutput*>(output);

	if (tileOutput != NULL)
	{
		path tileDir;

		if (tileOutput->outputDirectory().size() == 0)
		{
			QString outputDirectory = QString::fromStdString(project_->projectPath().string());
			outputDirectory.replace(".xml", "", Qt::CaseInsensitive);

			tileDir = path(outputDirectory.toStdString());
			tileDir = tileDir / tileOutput->name().toStdString();
		}
		else
		{
			tileDir = path(tileOutput->outputDirectory().toStdString());
			tileDir = tileDir / tileOutput->name().toStdString();
		}

		// clean out the directory
		for (int z = 0; z <= 25; ++z)
		{
			path zoomDir = tileDir / QString::number(z).toStdString();
			std::error_code ec;
			remove_all(zoomDir, ec);
		}

		int tileSize = tileOutput->tileSizePixels();

		double latitude = 42.608089;
		double lon = -71.571152;
		double distDeg = 0.1;

		auto ll0 = std::pair<double, double>(lon - distDeg, latitude + distDeg);
		auto ll1 = std::pair<double, double>(lon + distDeg, latitude - distDeg);

		unsigned int maxThreads = std::thread::hardware_concurrency();

		Render render1x(project_, 1);
		Render render2x(project_, 2);

		for (int z = tileOutput->minZoom(); z <= tileOutput->maxZoom(); ++z)
		{
			std::string zoomStr = QString::number(z).toStdString();
			path zoomDir = tileDir / zoomStr;

			create_directories(zoomDir);

			std::pair<double, double> px0 = fromLLtoPixel(tileSize, ll0, z);
			std::pair<double, double> px1 = fromLLtoPixel(tileSize, ll1, z);

			for (int x = int(px0.first / tileSize); x < int(px1.first / tileSize) + 1; ++x)
			{
				// Validate x co - ordinate
				if ((x < 0) || (x >= pow(2, z)))
					continue;

				// check if we have directories in place
				std::string str_x = QString::number(x).toStdString();

				if (exists(tileDir / zoomStr / str_x) == false)
					create_directories(tileDir / zoomStr / str_x);

				int yEnd = int(px1.second / tileSize) + 1;

				for (int y = int(px0.second / tileSize); y < yEnd; y += 1)
				{

					// Validate x co - ordinate
					if ((y < 0) || (y >= pow(2, z)))
						continue;

					std::string str_y = QString::number(y).toStdString();

					// Submit tile to be rendered into the queue
					//t = (name, tile_uri, x, y, z)
					if (tileOutput->resolution1x())
					{
						path tile_uri1x = tileDir / zoomStr / str_x / (str_y + ".png");
						RenderTile(render1x, tile_uri1x, tileSize, 1, x, y, z);
						//threads[threadI] = std::thread([this, tile_uri1x, tileSize, x, y, z](Render &render) { RenderTile(render, tile_uri1x, tileSize, 1, x, y, z); }, render1x);
						//threads[threadI].join();
					}

					if (tileOutput->resolution2x())
					{
						path tile_uri2x = tileDir / zoomStr / str_x / (str_y + "@2x.png");
						RenderTile(render2x, tile_uri2x, tileSize, 2, x, y, z);
						//threads[threadI] = std::thread([this, tile_uri2x, tileSize, x, y, z](Render &render) { RenderTile(render, tile_uri2x, tileSize, 2, x, y, z); }, *render2x[threadI]);
						//threads[threadI].join();
					}
				}
			}
		}
	}
}


void OutputTab::RenderTile(Render &render, const path &imagePath, int tileSize, int resolutionScale, int x, int y, int z)
{
	// Calculate pixel positions of bottom - left & top - right
	std::pair<int,int> p0(x * tileSize, (y + 1) * tileSize);

	std::pair<int, int> p1((x + 1) * tileSize, y * tileSize);

	// Convert to LatLong (EPSG:4326)
	std::pair<double, double> l0 = fromPixelToLL(tileSize, p0, z);
	std::pair<double, double> l1 = fromPixelToLL(tileSize,p1, z);

	// Convert to map projection (e.g. mercator co-ords EPSG:900913)
	// c0 = self.prj.forward(mapnik.Coord(l0[0], l0[1]));
	double x0, y0;
	project_->convertDataToMap(l0.first, l0.second, &x0, &y0);

	// c1 = self.prj.forward(mapnik.Coord(l1[0], l1[1]));
	double x1, y1;
	project_->convertDataToMap(l1.first, l1.second, &x1, &y1);

	render.SetupZoomBoundingBox(tileSize*resolutionScale, tileSize*resolutionScale, x0, x1, y0, y1);

	QImage img = render.RenderImage();
	img.save(QString::fromStdString(imagePath.string()));
}

std::pair<double, double> OutputTab::fromPixelToLL(int tileSize, std::pair<double, double> px, int zoom)
{
	std::vector<double> Bc;
	std::vector<double> Cc;
	std::vector<double> zc;
	std::vector<double> Ac;

	const double PI = std::atan(1.0) * 4;

	int c = 256;
	for (int z = 0; z <= 20; ++z)
	{
		double e = c / 2;

		Bc.push_back(c / 360.0);
		Cc.push_back(c / (2 * PI));
		zc.push_back(e);
		Ac.push_back(c);
		c *= 2;
	}

	const double RAD_TO_DEG = 180.0 / PI;

	double e = zc[zoom];
	double f = (px.first - e) / Bc[zoom];
	double g = (px.second - e) / -Cc[zoom];
	double h = RAD_TO_DEG * (2 * atan(exp(g)) - 0.5 * PI);
	return std::pair<double, double>(f, h);
}

std::pair<double, double> OutputTab::fromLLtoPixel(int tileSize, std::pair<double, double> ll, int zoom)
{
	std::vector<double> Bc;
	std::vector<double> Cc;
	std::vector<double> zc;
	std::vector<double> Ac;

	const double PI = std::atan(1.0) * 4;

	int c = 256;
	for (int z = 0; z <= 20; ++z)
	{
		double e = c / 2;

		Bc.push_back(c / 360.0);
		Cc.push_back(c / (2 * PI));
		zc.push_back(e);
		Ac.push_back(c);
		c *= 2;
	}

	const double DEG_TO_RAD = PI / 180.0;

	double d = zc[zoom];
	double e = round(d + ll.first * Bc[zoom]);
	double f = std::min<double>( std::max<double>(sin(DEG_TO_RAD * ll.second), -0.9999), 0.9999);
	double g = round(d + 0.5*log((1 + f) / (1 - f))*-Cc[zoom]);
	return std::pair<double, double>(e, g);
}




