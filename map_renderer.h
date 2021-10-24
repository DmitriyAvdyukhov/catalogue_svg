#pragma once
#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"


#include <tuple>
#include <algorithm>



namespace sphere_projector
{
	inline const double EPSILON = 1e-6;

	bool IsZero(double value);

	class SphereProjector
	{
	public:

		SphereProjector() = default;

		template <typename PointInputIt>
		SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

		svg::Point operator()(geo::Coordinates coords) const;

	private:
		double padding_ = 0.;
		double min_lon_ = 0.;
		double max_lat_ = 0.;
		double zoom_coeff_ = 0.;
	};

	template <typename PointInputIt>
	SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding)
		: padding_(padding)
	{
		if (points_begin == points_end)
		{
			return;
		}

		const auto [left_it, right_it]
			= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs)
				{
					return lhs.lng < rhs.lng;
				});
		min_lon_ = left_it->lng;
		const double max_lon = right_it->lng;

		const auto [bottom_it, top_it]
			= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs)
				{
					return lhs.lat < rhs.lat;
				});
		const double min_lat = bottom_it->lat;
		max_lat_ = top_it->lat;

		std::optional<double> width_zoom;
		if (!IsZero(max_lon - min_lon_))
		{
			width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
		}

		std::optional<double> height_zoom;
		if (!IsZero(max_lat_ - min_lat))
		{
			height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
		}

		if (width_zoom && height_zoom)
		{
			zoom_coeff_ = std::min(*width_zoom, *height_zoom);
		}
		else if (width_zoom)
		{
			zoom_coeff_ = *width_zoom;
		}
		else if (height_zoom)
		{
			zoom_coeff_ = *height_zoom;
		}
	}

}//namespace

namespace renderer
{
	struct TempDocument
	{
		std::vector<svg::Polyline>shap_buses;
		std::vector<svg::Text>name_bus;
		std::map<std::string, svg::Circle> circle_stops;
		std::map<std::string, std::pair<svg::Text, svg::Text>> name_stops;
	};

	struct RenderSettings
	{
		double width = 0.;
		double height = 0.;
		double padding = 0.;
		double line_width = 0.;
		double stop_radius = 0.;

		double bus_label_font_size = 0.;
		geo::Coordinates bus_label_offset{};

		double stop_label_font_size = 0.;
		geo::Coordinates stop_label_offset{};

		svg::Color underlayer_color{};
		double underlayer_width = 0.;

		std::vector<svg::Color> color_palette{};
	};

	struct BusSvg
	{
		svg::Polyline route_bus;
		std::vector<svg::Text> name_bus;
		std::vector<std::pair<std::string, svg::Circle>> circle_stops;
		std::vector<std::tuple<std::string, std::pair<svg::Text, svg::Text>>> name_stops;
	};

	class MapRenderer
	{
	public:
		explicit MapRenderer(const json::Dict& render_settings, const transport_catalogue::TransportCatalogue& tc);

		svg::Color AddColor(const json::Node& node);

		inline svg::Polyline AddRouteBus(const transport_catalogue::Bus& bus, const svg::Color& color);

		inline std::vector<svg::Text> AddNameBus(const transport_catalogue::Bus& bus, const svg::Color& color);

		inline std::vector<std::tuple<std::string, std::pair<svg::Text, svg::Text>>> AddNameStops(const transport_catalogue::Bus& bus);

		inline std::vector<std::pair<std::string, svg::Circle>> AddCircleStops(const transport_catalogue::Bus& bus);

		inline void PushBusSvg(const transport_catalogue::Bus& bus, const svg::Color& color);

		void SetSphereProjector(const transport_catalogue::TransportCatalogue& tc);

		void AddBusSvg(const transport_catalogue::TransportCatalogue& tc);

		inline TempDocument PreparationDocument(const std::vector<BusSvg>& buses)const;

		std::string DocumentMapToString()const;

	private:

		RenderSettings settings_;
		sphere_projector::SphereProjector s_;
		std::vector<BusSvg>buses_;
	};

}//namespace