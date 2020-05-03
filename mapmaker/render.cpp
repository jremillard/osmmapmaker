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

Render::Render(Project *project, double leftLinear, double bottomLinear, double pixelResolution, int imageWithPixels, int imageHeightPixels)
{
	using namespace mapnik;

	if (mapnikInit == false)
	{	//datasource_cache::instance().register_datasources("plugins/input/");

		datasource_cache::instance().register_datasources("C:\\Remillard\\Documents\\osmmapmaker\\vcpkg\\installed\\x64-windows\\debug\\bin", true);

		freetype_engine::register_font("fonts/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf");
		mapnikInit = true;
	}

	const std::string srs_merc = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over";

	map_ = Map(imageWithPixels, imageHeightPixels);

	map_.set_background(parse_color("blue"));
	map_.set_srs(srs_merc);

	path renderDbPath = project->renderDatabasePath();
	QString nativePath = QString::fromStdWString(renderDbPath.native());

	feature_type_style roads34_style;

	{
		rule r;
		r.set_filter(parse_expression("[highway]='track'"));

		{
			line_symbolizer line_sym;
			put(line_sym, keys::stroke, color(0xFF, 0xFF, 0xFF));
			put(line_sym, keys::stroke_width, 5.0);
			put(line_sym, keys::stroke_linecap, ROUND_CAP);
			put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
			r.append(std::move(line_sym));

		}

		roads34_style.add_rule(std::move(r));
	}

	map_.insert_style("smallroads", std::move(roads34_style));

	{
		parameters p;
		p["type"] = "sqlite";
		p["file"] = nativePath.toUtf8().constBegin();
		p["table"] = "highway_v";
		p["geometry_field"] = "geom";
		p["wkb_format"] = "generic";
		p["key_field"] = "id";
		p["auto_index"] = "true";
		p["use_spatial_index"] = "true";

		layer lyr("smallroads");

		lyr.set_datasource(datasource_cache::instance().create(p));

		lyr.set_srs("+proj=longlat +datum=WGS84 +no_defs");

		lyr.add_style("smallroads");

		map_.add_layer(lyr);
	}
}

QImage Render::RenderImage()
{
	//m.zoom_to_box(box2d<double>(-8024477.28459, 5445190.38849, -7381388.20071, 5662941.44855));
	map_.zoom_all();

	box2d<double> bb = map_.get_current_extent();

	image_rgba8 buf(map_.width(), map_.height());

	agg_renderer<image_rgba8> ren(map_, buf);

	ren.apply();

	save_to_file(buf, "c:\\remillard\\documents\\demo.png", "png");

	save_map(map_, "c:\\remillard\\documents\\map.xml");

	QImage img = QImage(map_.width(), map_.height(), QImage::Format_ARGB32);

	for (std::size_t row = 0; row < map_.height(); ++row)
	{
		image_rgba8::pixel_type const* rowPtr = buf.get_row(row);

		memcpy(img.scanLine(row), rowPtr, map_.width()*sizeof(image_rgba8::pixel_type));
	}

	img.save("c:\\remillard\\documents\\demo2.png");

	return img;
}
