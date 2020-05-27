#include "outputTab.h"
#include "ui_outputTab.h"

#include <filesystem>
#include <algorithm>
#include <math.h>

OutputTab::OutputTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OutputTab)
{
	ui->setupUi(this);

	ui->tilePath->setText("c:\\remillard\\documents\\tiles");
	ui->minZoom->setValue(13);
	ui->maxZoom->setValue(18);

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
}

void OutputTab::on_generate_clicked()
{
	path tileDir(ui->tilePath->text().toStdString());

	Render render(project_);

	for (int z = ui->minZoom->value(); z <= ui->maxZoom->value(); ++z)
	{
		path zoomDir = tileDir / QString::number(z).toStdString();
		std::error_code ec;
		remove(zoomDir, ec);
	}

	double latitude = 42.608089;
	double lon = -71.571152;
	double distDeg = 0.1;

	auto ll0 = std::pair<double, double>(lon-distDeg, latitude+distDeg);
	auto ll1 = std::pair<double, double>(lon+distDeg, latitude-distDeg);

	for (int z = ui->minZoom->value(); z <= ui->maxZoom->value(); ++z)
	{
		std::string zoomStr = QString::number(z).toStdString();
		path zoomDir = tileDir / zoomStr;

		create_directories(zoomDir);

		std::pair<double,double> px0 = fromLLtoPixel(ll0, z);
		std::pair<double,double> px1 = fromLLtoPixel(ll1, z);

		for ( int x = int(px0.first / 256.0); x < int(px1.first / 256.0) + 1; ++x)
		{
			// Validate x co - ordinate
			if ( (x < 0) || (x >= pow(2,z)))
				continue;

			// check if we have directories in place
			std::string str_x = QString::number(x).toStdString();

			if (exists(tileDir / zoomStr / str_x) == false)
				create_directories(tileDir / zoomStr / str_x);

			for (int y = int(px0.second / 256.0); y < int(px1.second / 256.0) + 1; ++y)
			{	
				// Validate x co - ordinate
				if ((y < 0) || (y >= pow(2,z)))
					continue;

				std::string str_y = QString::number(y).toStdString();

				path tile_uri = tileDir / zoomStr / str_x / (str_y + ".png");

				// Submit tile to be rendered into the queue
				//t = (name, tile_uri, x, y, z)
				RenderTile(render, tile_uri, x, y, z);
			}
		}
	}
}

void OutputTab::RenderTile(Render &render, const path &imagePath, int x, int y, int z)
{
	// Calculate pixel positions of bottom - left & top - right
	std::pair<int,int> p0(x * 256, (y + 1) * 256);

	std::pair<int, int> p1((x + 1) * 256, y * 256);

	// Convert to LatLong (EPSG:4326)
	std::pair<double, double> l0 = fromPixelToLL(p0, z);
	std::pair<double, double> l1 = fromPixelToLL(p1, z);

	// Convert to map projection (e.g. mercator co-ords EPSG:900913)
	// c0 = self.prj.forward(mapnik.Coord(l0[0], l0[1]));
	double x0, y0;
	project_->convertDataToMap(l0.first, l0.second, &x0, &y0);

	// c1 = self.prj.forward(mapnik.Coord(l1[0], l1[1]));
	double x1, y1;
	project_->convertDataToMap(l1.first, l1.second, &x1, &y1);

	render.SetupZoomBoundingBox(256, 256, x0, x1, y0, y1);

	QImage img = render.RenderImage();
	img.save(QString::fromStdString(imagePath.string()));
}

std::pair<double, double> OutputTab::fromPixelToLL(std::pair<double, double> px, int zoom)
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

std::pair<double, double> OutputTab::fromLLtoPixel(std::pair<double, double> ll, int zoom)
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




