#include <render.h>

#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>

#include <mapnik/save_map.hpp>

using namespace mapnik;

static bool mapnikInit = false;

Render::Render(Project *project)
{
	using namespace mapnik;

	if (mapnikInit == false)
	{	//datasource_cache::instance().register_datasources("plugins/input/");

		datasource_cache::instance().register_datasources("C:\\Remillard\\Documents\\osmmapmaker\\vcpkg\\installed\\x64-windows\\debug\\bin", true);

		freetype_engine::register_font("fonts/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf");
		mapnikInit = true;
	}

	const std::string srs_merc = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over";

	map_ = Map(100, 100);

	map_.set_background(parse_color("green"));
	map_.set_srs(srs_merc);

	path renderDbPath = project->renderDatabasePath();
	QString nativePath = QString::fromStdWString(renderDbPath.native());

	size_t styleIndex = 0;
	for (auto projectLayer : project->styleLayers())
	{	
		parameters p;
		p["type"] = "sqlite";
		p["file"] = nativePath.toUtf8().constBegin();
		p["table"] = projectLayer->key().toStdString() + "_v";
		p["geometry_field"] = "geom";
		p["wkb_format"] = "generic";
		p["key_field"] = "id";
		p["auto_index"] = "true";
		p["use_spatial_index"] = "true";

		QString layerName = QString("%1-%2").arg(projectLayer->key()).arg(styleIndex);

		layer lyr(layerName.toStdString());

		lyr.set_datasource(datasource_cache::instance().create(p));

		lyr.set_srs("+proj=longlat +datum=WGS84 +no_defs");

		switch (projectLayer->layerType())
		{
			case ST_LABEL:
			{
				break;
			}

			case ST_POINT:
			{
				break;
			}

			case ST_LINE:
			{
				std::vector<QString> names = projectLayer->subLayerNames();

				for (int subLayerIndex = 0; subLayerIndex < names.size(); ++subLayerIndex)
				{
					Line line = projectLayer->subLayerLine(subLayerIndex);

					if (line.casingWidth_ > 0)
					{
						feature_type_style style;

						rule r;
						//r.set_filter(parse_expression("[highway]='track'"));

						line_symbolizer line_sym;
						put(line_sym, keys::stroke, color(line.casingColor_.red(), line.casingColor_.green(), line.casingColor_.blue()));
						put(line_sym, keys::stroke_width, line.width_ + line.casingWidth_*2.0);
						//put(line_sym, keys::stroke_linecap, ROUND_CAP);
						//put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
						put(line_sym, keys::stroke_opacity, line.opacity_);
						put(line_sym, keys::smooth, line.smooth_);

						r.append(std::move(line_sym));

						style.add_rule(std::move(r));

						QString styleLayer = QString("%1-%2-%3-Cas").arg(projectLayer->key()).arg(styleIndex).arg(subLayerIndex);

						map_.insert_style(styleLayer.toStdString(), std::move(style));

						lyr.add_style(styleLayer.toStdString());

					}

					feature_type_style style;

					rule r;
					//r.set_filter(parse_expression("[highway]='track'"));

					line_symbolizer line_sym;
					put(line_sym, keys::stroke, color(line.color_.red(), line.color_.green(), line.color_.blue()));
					put(line_sym, keys::stroke_width, line.width_);
					//put(line_sym, keys::stroke_linecap, ROUND_CAP);
					//put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
					put(line_sym, keys::stroke_opacity, line.opacity_);
					put(line_sym, keys::smooth, line.smooth_);

					r.append(std::move(line_sym));

					style.add_rule(std::move(r));

					QString styleLayer = QString("%1-%2-%3").arg(projectLayer->key()).arg(styleIndex).arg(subLayerIndex);

					map_.insert_style(styleLayer.toStdString(), std::move(style));

					lyr.add_style(styleLayer.toStdString());

					++subLayerIndex;
				}

				break;
			}

			case ST_AREA:
			{
				break;
			}
		}

		map_.add_layer(lyr);
		++styleIndex;
	}
}

void Render::GetBoundingBox(double *centerX, double *centerY, double *pixelResolution)
{

	*pixelResolution = map_.scale();

	box2d<double> box = map_.get_current_extent();

	*centerX = box.center().x;
	*centerY = box.center().y;
}


QImage Render::RenderImage(int imageWithPixels, int imageHeightPixels, double centerX, double centerY, double pixelResolution)
{
	map_.resize(imageWithPixels, imageHeightPixels);

	if (pixelResolution <= 0)
	{
		map_.zoom_all();
	}
	else
	{
		double widthLin = (double)imageWithPixels * pixelResolution;
		double heightLin = (double)imageHeightPixels * pixelResolution;

		box2d<double> box(centerX-widthLin/2.0, centerY - heightLin / 2.0, centerX + widthLin / 2.0, centerY + heightLin / 2.0);
		map_.zoom_to_box(box);
	}

	box2d<double> bb = map_.get_current_extent();

	image_rgba8 buf(map_.width(), map_.height());

	agg_renderer<image_rgba8> ren(map_, buf);

	ren.apply();

	//save_to_file(buf, "c:\\remillard\\documents\\demo.png", "png");

	//save_map(map_, "c:\\remillard\\documents\\map.xml");

	QImage img = QImage(map_.width(), map_.height(), QImage::Format_ARGB32);

	for (std::size_t row = 0; row < buf.height(); ++row)
	{
		image_rgba8::pixel_type * rowPtr = buf.get_row(row);

		// probably not needed on OS-X... mapnik didn't use the native byte ordering, the used disk format of 
		// libpng, need to sort that out here.
		for (std::size_t col = 0; col < buf.width(); ++col)
		{
			image_rgba8::pixel_type pixel = rowPtr[col] & 0x00FFFFFF;
			pixel = _byteswap_ulong(pixel);
			rowPtr[col] = (rowPtr[col] & 0xFF000000) | (pixel >> 8);
		}

		memcpy(img.scanLine(row), rowPtr, map_.width()*sizeof(image_rgba8::pixel_type));
	}

	//img.save("c:\\remillard\\documents\\demo2.png");

	return img;
}
