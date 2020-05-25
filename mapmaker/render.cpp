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
	project->createViews();

	using namespace mapnik;

	if (mapnikInit == false)
	{	//datasource_cache::instance().register_datasources("plugins/input/");

		datasource_cache::instance().register_datasources("C:\\Remillard\\Documents\\osmmapmaker\\vcpkg\\installed\\x64-windows\\debug\\bin", true);

		path font = project->assetDirectory() / "DejaVuSans.ttf";

		freetype_engine::register_font(font.string());
		mapnikInit = true;
	}

	map_ = Map(100, 100);

	map_.set_background(color( project->backgroundColor().red(), project->backgroundColor().green(), project->backgroundColor().blue()));
	map_.set_srs(project->mapSRS());

	path renderDbPath = project->renderDatabasePath();
	QString nativePath = QString::fromStdWString(renderDbPath.native());

	size_t styleIndex = 0;

	std::vector< StyleLayer*> layers = project->styleLayers();

	for (auto projectLayerI = layers.rbegin(); projectLayerI != layers.rend(); ++projectLayerI)
	{	
		StyleLayer *projectLayer = *projectLayerI;;

		parameters p;
		p["type"] = "sqlite";
		p["file"] = "render.sqlite";
		p["base"] = project->assetDirectory().string();
		p["table"] = projectLayer->virtualSQLTableName().toStdString();
		p["geometry_field"] = "geom";
		p["wkb_format"] = "generic";
		p["key_field"] = "id";
		p["auto_index"] = "false";
		p["index_table"] = "entitySpatialIndex";
		p["use_spatial_index"] = "true";

		QString layerName = QString("%0-%1").arg(projectLayer->key()).arg(styleIndex);

		layer lyr(layerName.toStdString());

		lyr.set_datasource(datasource_cache::instance().create(p));

		lyr.set_srs(project->dataSRS());

		std::vector<QString> names = projectLayer->subLayerNames();

		QString exclusiveExpression;

		for (int subLayerIndex = 0; subLayerIndex < names.size(); ++subLayerIndex)
		{
			StyleSelector selector = projectLayer->subLayerSelectors(subLayerIndex);

			QString layerExp = selector.mapniKExpression();
			if (exclusiveExpression.isEmpty() == false)
			{
				layerExp += " and not " + exclusiveExpression;
			}

			exclusiveExpression = layerExp;

			auto expression = parse_expression(layerExp.toStdString());

			switch (projectLayer->layerType())
			{
			case ST_POINT:
			{
				break;
			}

			case ST_LINE:
			{
				Line line = projectLayer->subLayerLine(subLayerIndex);
				if (line.visible_ == false)
					continue;

				if (line.casingWidth_ > 0)
				{
					feature_type_style style;

					rule r;
					r.set_filter(expression);

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
				r.set_filter(expression);

				line_symbolizer line_sym;
				put(line_sym, keys::stroke, color(line.color_.red(), line.color_.green(), line.color_.blue()));
				put(line_sym, keys::stroke_width, line.width_);
				//put(line_sym, keys::stroke_linecap, ROUND_CAP);
				//put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
				put(line_sym, keys::stroke_opacity, line.opacity_);
				put(line_sym, keys::smooth, line.smooth_);

				r.append(std::move(line_sym));

				style.add_rule(std::move(r));

				QString styleLayer = QString("%0-%1-%2").arg(projectLayer->key()).arg(styleIndex).arg(subLayerIndex);

				map_.insert_style(styleLayer.toStdString(), std::move(style));

				lyr.add_style(styleLayer.toStdString());


				break;
			}

			case ST_AREA:
			{
				Area area = projectLayer->subLayerArea(subLayerIndex);
				if (area.visible_ == false)
					continue;

				if (area.opacity_ > 0)
				{
					feature_type_style style;

					rule r;
					r.set_filter(expression);

					polygon_symbolizer area_sym;

					put(area_sym, keys::fill, color(area.color_.red(), area.color_.green(), area.color_.blue()));
					put(area_sym, keys::fill_opacity, area.opacity_);

					if (area.casingWidth_ > 0)
						put(area_sym, keys::gamma, 0);
					else
						put(area_sym, keys::gamma, 1);

					r.append(std::move(area_sym));

					style.add_rule(std::move(r));

					QString styleLayer = QString("%0-%1-%2-fill").arg(projectLayer->key()).arg(styleIndex).arg(subLayerIndex);

					map_.insert_style(styleLayer.toStdString(), std::move(style));

					lyr.add_style(styleLayer.toStdString());
				}

				if (area.fillImage_.isEmpty() == false)
				{
					feature_type_style style;

					rule r;
					r.set_filter(expression);

					polygon_pattern_symbolizer area_pattern;

					// base path on map property doesn't work for C++, seems to be just an xml thing, put in full path for everything.
					path fillImagePath = project->assetDirectory() / area.fillImage_.toStdString();
					put(area_pattern, keys::file, fillImagePath.string());

					put(area_pattern, keys::opacity, area.fillImageOpacity_);

					r.append(std::move(area_pattern));

					style.add_rule(std::move(r));

					QString styleLayer = QString("%0-%1-%2-imageFill").arg(projectLayer->key()).arg(styleIndex).arg(subLayerIndex);

					map_.insert_style(styleLayer.toStdString(), std::move(style));

					lyr.add_style(styleLayer.toStdString());

				}

				if (area.casingWidth_ > 0)
				{
					feature_type_style style;

					rule r;
					r.set_filter(expression);

					line_symbolizer line_sym;
					put(line_sym, keys::stroke, color(area.casingColor_.red(), area.casingColor_.green(), area.casingColor_.blue()));
					put(line_sym, keys::stroke_width, area.casingWidth_);
					put(line_sym, keys::stroke_opacity, area.opacity_);
					put(line_sym, keys::smooth, 0.0);

					r.append(std::move(line_sym));

					style.add_rule(std::move(r));

					QString styleLayer = QString("%0-%1-%2-Cas").arg(projectLayer->key()).arg(styleIndex).arg(subLayerIndex);

					map_.insert_style(styleLayer.toStdString(), std::move(style));

					lyr.add_style(styleLayer.toStdString());

				}

				break;
			}
			}
		}

		map_.add_layer(lyr);
		++styleIndex;
	}

	// render labels
	for (auto projectLayerI = layers.rbegin(); projectLayerI != layers.rend(); ++projectLayerI)
	{
		StyleLayer *projectLayer = *projectLayerI;;

		bool layerHasLable = false;

		std::vector<QString> names = projectLayer->subLayerNames();

		for (int subLayerIndex = 0; subLayerIndex < names.size(); ++subLayerIndex)
		{
			Label label = projectLayer->label(subLayerIndex);
			if (label.visible_)
			{
				layerHasLable = true;
				break;
			}
		}

		if (layerHasLable == false)
		{
			++styleIndex;
			continue;
		}

		parameters p;
		p["type"] = "sqlite";
		p["file"] = "render.sqlite";
		p["base"] = project->assetDirectory().string();
		p["table"] = projectLayer->virtualSQLTableName().toStdString();
		p["geometry_field"] = "geom";
		p["wkb_format"] = "generic";
		p["key_field"] = "id";
		p["auto_index"] = "false";
		p["index_table"] = "entitySpatialIndex";
		p["use_spatial_index"] = "true";

		QString layerName = QString("%0-%1-Label").arg(projectLayer->key()).arg(styleIndex);

		layer lyr(layerName.toStdString());

		lyr.set_datasource(datasource_cache::instance().create(p));

		lyr.set_srs(project->dataSRS());

		QString exclusiveExpression;
		for (int subLayerIndex = 0; subLayerIndex < names.size(); ++subLayerIndex)
		{
			StyleSelector selector = projectLayer->subLayerSelectors(subLayerIndex);

			QString layerExp = selector.mapniKExpression();
			if (exclusiveExpression.isEmpty() == false)
			{
				layerExp += " and not " + exclusiveExpression;
			}

			exclusiveExpression = layerExp;

			auto expression = parse_expression(layerExp.toStdString());

			Label label = projectLayer->label(subLayerIndex);

			switch (projectLayer->layerType())
			{
			case ST_POINT:
			{
				dot_symbolizer dot;
				break;
			}

			case ST_LINE:
			{
				Line line = projectLayer->subLayerLine(subLayerIndex);
				if (line.visible_ == false)
					continue;

				feature_type_style style;

				if (label.visible_)
				{
					rule r;
					r.set_filter(expression);

					text_symbolizer text_sym;
					text_placements_ptr placement_finder = std::make_shared<text_placements_dummy>();

					placement_finder->defaults.format_defaults.face_name = "DejaVu Sans Book";
					placement_finder->defaults.format_defaults.text_size = label.height_;
					placement_finder->defaults.format_defaults.fill = color(label.color_.red(), label.color_.green(), label.color_.blue());
					placement_finder->defaults.format_defaults.halo_fill = color(label.haloColor_.red(), label.haloColor_.green(), label.haloColor_.blue());
					placement_finder->defaults.format_defaults.halo_radius = label.haloSize_;

					placement_finder->defaults.expressions.label_spacing = label.lineLaxSpacing_;
					placement_finder->defaults.expressions.largest_bbox_only = false; // line multipolygon, get labels on every line.

					placement_finder->defaults.set_format_tree(std::make_shared<mapnik::formatting::text_node>(parse_expression(label.text_.toStdString())));

					placement_finder->defaults.expressions.label_placement = enumeration_wrapper(LINE_PLACEMENT);

					put<text_placements_ptr>(text_sym, keys::text_placements_, placement_finder);

					r.append(std::move(text_sym));

					style.add_rule(std::move(r));

					QString styleLayer = QString("%0-%1-%2-Label").arg(projectLayer->key()).arg(styleIndex).arg(subLayerIndex);

					map_.insert_style(styleLayer.toStdString(), std::move(style));
					lyr.add_style(styleLayer.toStdString());
				}
			}
			break;

			case ST_AREA:
			{
				Area area = projectLayer->subLayerArea(subLayerIndex);
				if (area.visible_ == false)
					continue;

				if (label.visible_)
				{
					feature_type_style style;

					rule r;
					r.set_filter(expression);

					text_symbolizer text_sym;
					text_placements_ptr placement_finder = std::make_shared<text_placements_dummy>();

					placement_finder->defaults.format_defaults.face_name = "DejaVu Sans Book";
					placement_finder->defaults.format_defaults.text_size = label.height_;
					placement_finder->defaults.format_defaults.fill = color(label.color_.red(), label.color_.green(), label.color_.blue());
					placement_finder->defaults.format_defaults.halo_fill = color(label.haloColor_.red(), label.haloColor_.green(), label.haloColor_.blue());
					placement_finder->defaults.format_defaults.halo_radius = label.haloSize_;

					placement_finder->defaults.layout_defaults.wrap_width = label.maxWrapWidth_;

					placement_finder->defaults.expressions.label_placement = enumeration_wrapper(INTERIOR_PLACEMENT);

					placement_finder->defaults.set_format_tree(std::make_shared<mapnik::formatting::text_node>(parse_expression(label.text_.toStdString())));

					put<text_placements_ptr>(text_sym, keys::text_placements_, placement_finder);

					r.append(std::move(text_sym));

					style.add_rule(std::move(r));

					QString styleLayer = QString("%0-%1-%2-Label").arg(projectLayer->key()).arg(styleIndex).arg(subLayerIndex);

					map_.insert_style(styleLayer.toStdString(), std::move(style));
					lyr.add_style(styleLayer.toStdString());
				}
			}
			break;


			}
		}

		map_.add_layer(lyr);
	}

	//path mapnikXML = project->assetDirectory() / "mapnik.xml";
	//save_map(map_, mapnikXML.string());
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

	return img;
}
