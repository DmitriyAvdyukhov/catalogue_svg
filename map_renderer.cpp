#include "map_renderer.h"


namespace sphere_projector
{
	bool IsZero(double value)
	{
		return std::abs(value) < EPSILON;
	}
	svg::Point SphereProjector::operator()(geo::Coordinates coords) const
	{
		return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
				(max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
	}

}//namespace sphere_projector



namespace renderer
{
	MapRenderer::MapRenderer(const json::Dict& render_settings, const transport_catalogue::TransportCatalogue& tc)
	{
		settings_.width = render_settings.at("width").AsDouble();
		settings_.height = render_settings.at("height").AsDouble();
		settings_.padding = render_settings.at("padding").AsDouble();
		settings_.line_width = render_settings.at("line_width").AsDouble();
		settings_.stop_radius = render_settings.at("stop_radius").AsDouble();
		settings_.bus_label_font_size = render_settings.at("bus_label_font_size").AsDouble();
		settings_.bus_label_offset.lat = render_settings.at("bus_label_offset").AsArray().front().AsDouble();
		settings_.bus_label_offset.lng = render_settings.at("bus_label_offset").AsArray().back().AsDouble();
		settings_.stop_label_font_size = render_settings.at("stop_label_font_size").AsDouble();
		settings_.stop_label_offset.lat = render_settings.at("stop_label_offset").AsArray().front().AsDouble();
		settings_.stop_label_offset.lng = render_settings.at("stop_label_offset").AsArray().back().AsDouble();
		settings_.underlayer_width = render_settings.at("underlayer_width").AsDouble();
		settings_.underlayer_color = AddColor(render_settings.at("underlayer_color"));
		json::Array palette = render_settings.at("color_palette").AsArray();
		for (auto it = palette.begin(); it != palette.end(); ++it)
		{
			settings_.color_palette.push_back(AddColor(*it));
		}

		AddBusSvg(tc);
	}
	//------------------Add-----------------------

	svg::Color MapRenderer::AddColor(const json::Node& node)
	{
		{
			svg::Color color;
			if (node.IsArray())
			{
				if (node.AsArray().size() == 3)
				{
					svg::Rgb rgb;
					rgb.red = node.AsArray()[0].AsInt();
					rgb.green = node.AsArray()[1].AsInt();
					rgb.blue = node.AsArray()[2].AsInt();
					color = rgb;
				}
				if (node.AsArray().size() == 4)
				{
					svg::Rgba rgba;
					rgba.red = node.AsArray()[0].AsInt();
					rgba.green = node.AsArray()[1].AsInt();
					rgba.blue = node.AsArray()[2].AsInt();
					rgba.opacity = node.AsArray()[3].AsDouble();
					color = rgba;
				}
			}
			if (node.IsString())
			{
				color = node.AsString();
			}
			return color;
		}
	}

	svg::Polyline MapRenderer::AddRouteBus(const transport_catalogue::Bus& bus, const svg::Color& color)
	{
		svg::Polyline route_bus;
		route_bus.SetStrokeColor(color);
		route_bus.SetFillColor("none");
		route_bus.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		route_bus.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		route_bus.SetStrokeWidth(settings_.line_width);
		for (auto stop : bus.stops)
		{
			svg::Point point = s_({ stop->coordinates.lat, stop->coordinates.lng });
			route_bus.AddPoint(point);
		}
		return route_bus;
	}

	std::vector<svg::Text> MapRenderer::AddNameBus(const transport_catalogue::Bus& bus, const svg::Color& color)
	{
		{
			std::vector<svg::Text>result;
			svg::Text name_bus;
			svg::Text name_bus_substr;

			name_bus_substr = name_bus.SetPosition(s_({ bus.stops.front()->coordinates.lat, bus.stops.front()->coordinates.lng }));
			name_bus_substr = name_bus.SetOffset({ settings_.bus_label_offset.lat, settings_.bus_label_offset.lng });
			name_bus_substr = name_bus.SetFontSize(settings_.bus_label_font_size);
			name_bus_substr = name_bus.SetFontFamily("Verdana");
			name_bus_substr = name_bus.SetFontWeight("bold");
			name_bus_substr = name_bus.SetData(bus.name);
			name_bus.SetFillColor(color);
			name_bus_substr.SetFillColor(settings_.underlayer_color);
			name_bus_substr.SetStrokeColor(settings_.underlayer_color);
			name_bus_substr.SetStrokeWidth(settings_.underlayer_width);
			name_bus_substr.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			name_bus_substr.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			result.push_back(name_bus_substr);
			result.push_back(name_bus);

			if (!bus.is_roundtrip && bus.stops[(bus.stops.size() + 1) / 2 - 1] != bus.stops[0])
			{
				svg::Text name_bus;
				svg::Text name_bus_substr;

				name_bus_substr = name_bus.SetPosition(s_({ bus.stops[(bus.stops.size() + 1) / 2 - 1]->coordinates.lat
					, bus.stops[(bus.stops.size() + 1) / 2 - 1]->coordinates.lng }));
				name_bus_substr = name_bus.SetOffset({ settings_.bus_label_offset.lat, settings_.bus_label_offset.lng });
				name_bus_substr = name_bus.SetFontSize(settings_.bus_label_font_size);
				name_bus_substr = name_bus.SetFontFamily("Verdana");
				name_bus_substr = name_bus.SetFontWeight("bold");
				name_bus_substr = name_bus.SetData(bus.name);
				name_bus.SetFillColor(color);
				name_bus_substr.SetFillColor(settings_.underlayer_color);
				name_bus_substr.SetStrokeColor(settings_.underlayer_color);
				name_bus_substr.SetStrokeWidth(settings_.underlayer_width);
				name_bus_substr.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				name_bus_substr.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				result.push_back(name_bus_substr);
				result.push_back(name_bus);
			}
			return result;
		}
	}

	std::vector<std::tuple<std::string, std::pair<svg::Text, svg::Text>>> MapRenderer::AddNameStops(const transport_catalogue::Bus& bus)
	{
		{
			std::vector<std::tuple<std::string, std::pair<svg::Text, svg::Text>>> result;
			for (size_t i = 0; i < bus.stops.size() - 1; ++i)
			{
				svg::Text name_stop;
				svg::Text name_stop_substr;

				name_stop.SetPosition(s_({ bus.stops[i]->coordinates.lat, bus.stops[i]->coordinates.lng }));
				name_stop_substr.SetPosition(s_({ bus.stops[i]->coordinates.lat, bus.stops[i]->coordinates.lng }));

				name_stop.SetOffset({ settings_.stop_label_offset.lat, settings_.stop_label_offset.lng });
				name_stop_substr.SetOffset({ settings_.stop_label_offset.lat, settings_.stop_label_offset.lng });

				name_stop.SetFontSize(settings_.stop_label_font_size);
				name_stop_substr.SetFontSize(settings_.stop_label_font_size);

				name_stop.SetFontFamily("Verdana");
				name_stop_substr.SetFontFamily("Verdana");

				name_stop.SetData(bus.stops[i]->name);
				name_stop_substr.SetData(bus.stops[i]->name);

				name_stop_substr.SetFillColor(settings_.underlayer_color);
				name_stop_substr.SetStrokeColor(settings_.underlayer_color);
				name_stop_substr.SetStrokeWidth(settings_.underlayer_width);
				name_stop_substr.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				name_stop_substr.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				name_stop.SetFillColor("black");
				result.push_back({ bus.stops[i]->name ,{ name_stop, name_stop_substr } });
			}
			return result;
		}
	}

	std::vector<std::pair<std::string, svg::Circle>> MapRenderer::AddCircleStops(const transport_catalogue::Bus& bus)
	{
		std::vector<std::pair<std::string, svg::Circle>> result;
		for (size_t i = 0; i < bus.stops.size() - 1; ++i)
		{
			svg::Circle circle;
			circle.SetCenter(s_({ bus.stops[i]->coordinates.lat, bus.stops[i]->coordinates.lng }));
			circle.SetRadius(settings_.stop_radius);
			circle.SetFillColor("white");
			result.push_back({ bus.stops[i]->name ,circle });
		}
		return result;
	}

	void MapRenderer::AddBusSvg(const transport_catalogue::TransportCatalogue& tc)
	{
		SetSphereProjector(tc);

		std::vector<transport_catalogue::Bus>buses;
		for (const auto& bus : tc.GetRoute())
		{
			buses.push_back(bus);
		}

		sort(buses.begin(), buses.end(), [](const transport_catalogue::Bus& lhs, const transport_catalogue::Bus& rhs)
			{
				return lhs.name < rhs.name;
			});
		int j = 0;
		for (size_t i = 0; i < buses.size(); ++i)
		{
			if (buses[i].stops.size())
			{
				PushBusSvg(buses[i], settings_.color_palette[j]);

				if (j == settings_.color_palette.size() - 1)
				{
					j = -1;
				}
				++j;
			}
		}
	}

	//-----------------------Set------------------------------

	void MapRenderer::PushBusSvg(const transport_catalogue::Bus& bus, const svg::Color& color)
	{
		buses_.push_back({ AddRouteBus(bus,color) ,AddNameBus(bus, color) ,AddCircleStops(bus) ,AddNameStops(bus) });
	}

	void MapRenderer::SetSphereProjector(const transport_catalogue::TransportCatalogue& tc)
	{
		std::vector<geo::Coordinates> minlon_maxlat;
		for (const auto& bus : tc.GetRoute())
		{
			for (const auto& stop : bus.stops)
			{
				if (find(minlon_maxlat.begin(), minlon_maxlat.end(), stop->coordinates) == minlon_maxlat.end())
				{
					minlon_maxlat.push_back(stop->coordinates);
				}
			}
		}
		s_ = sphere_projector::SphereProjector(minlon_maxlat.begin(), minlon_maxlat.end(), settings_.width, settings_.height, settings_.padding);
	}

	//----------Document to string--------------------------------------

	inline TempDocument MapRenderer::PreparationDocument(const std::vector<BusSvg>& buses) const
	{
		TempDocument temp;
		for (const auto& bus : buses)
		{
			if (bus.name_stops.size())
			{
				temp.shap_buses.push_back(bus.route_bus);

				for (const auto& n_bus : bus.name_bus)
				{
					temp.name_bus.push_back(n_bus);
				}

				for (const auto& circle_stop : bus.circle_stops)
				{
					temp.circle_stops.insert({ circle_stop.first, circle_stop.second });
				}
				for (const auto& name_stop : bus.name_stops)
				{
					temp.name_stops.insert({ std::get<0>(name_stop),std::get<1>(name_stop) });
				}
			}
		}
		return temp;
	}

	std::string MapRenderer::DocumentMapToString() const
	{
		svg::Document doc;

		for (const auto& line : PreparationDocument(buses_).shap_buses)
		{
			doc.Add(line);
		}
		for (const auto& text : PreparationDocument(buses_).name_bus)
		{
			doc.Add(text);
		}

		for (const auto& circle : PreparationDocument(buses_).circle_stops)
		{
			doc.Add(circle.second);
		}

		for (const auto& stop : PreparationDocument(buses_).name_stops)
		{
			doc.Add(stop.second.second);
			doc.Add(stop.second.first);
		}

		std::stringstream map;
		doc.Render(map);
		return map.str();
	}

}//namespace renderer


