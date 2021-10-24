#include "request_handler.h"


using namespace transport_catalogue;
using namespace renderer;
using namespace geo;

RequestHandler::RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer) :db_(db), renderer_(renderer)
{}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const
{
	std::optional<BusStat> result;
	BusStat stat;
	BusPtr bus = db_.FindBus(bus_name);
	if (bus)
	{

		std::unordered_set<std::string_view> seen_stops;

		std::optional<Coordinates> prev_pos;

		for (auto stop : bus->stops)
		{
			++stat.total_stops;

			if (seen_stops.count(stop->name) == 0)
			{
				++stat.unique_stops;
				seen_stops.insert(stop->name);
			}

			if (prev_pos)
			{
				stat.route_length += ComputeDistance(*prev_pos, stop->coordinates);
			}

			prev_pos = stop->coordinates;
		}

		for (auto it = bus->stops.begin(); it != bus->stops.end() - 1; ++it)
		{
			std::pair<StopPtr, StopPtr> temp{ *it, *(it + 1) };
			stat.distance += db_.FindDistanceBetweenStops(temp);
		}
		result = stat;
	}
	return result;
}

StopInfo RequestHandler::GetBusesByStop(const std::string_view& stop_name) const
{
	StopInfo result;
	static const std::unordered_set<transport_catalogue::BusPtr> temp;
	if (!db_.FindStop(stop_name))
	{
		return StopInfo();
	}

	if (db_.GetStopToBuses().find(db_.FindStop(stop_name))->second.size())
	{
		result.buses_for_stop = db_.GetStopToBuses().find(db_.FindStop(stop_name))->second;
		result.about = "Ok"s;
	}
	else
	{
		result.buses_for_stop = temp;
		result.about = "no buses"s;
	}
	return result;
}

std::optional<std::string> RequestHandler::GetMap() const
{
	return renderer_.DocumentMapToString();
}


