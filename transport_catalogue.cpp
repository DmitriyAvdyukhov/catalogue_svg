#include "transport_catalogue.h"


using namespace transport_catalogue;

TransportCatalogue* TransportCatalogue::tc_ = nullptr;

StopPtr transport_catalogue::TransportCatalogue::FindStop(const std::string_view& stop) const noexcept
{
	if (name_stop_.count(stop) != 0)
	{
		return name_stop_.at(stop);
	}
	return nullptr;
}

BusPtr transport_catalogue::TransportCatalogue::FindBus(const std::string_view& buss) const noexcept
{
	if (name_bus_.count(buss) != 0)
	{
		return name_bus_.at(buss);
	}
	return nullptr;
}

int transport_catalogue::TransportCatalogue::FindDistanceBetweenStops(const std::pair<StopPtr, StopPtr>& stops) const noexcept
{
	if (dist_betw_stops_.find(stops) != dist_betw_stops_.end())
	{
		return dist_betw_stops_.at(stops);
	}
	else
	{
		std::pair<StopPtr, StopPtr> temp{ stops.second, stops.first };
		return dist_betw_stops_.at(temp);
	}
}

void transport_catalogue::TransportCatalogue::AddStop(std::pair<double, double> coordinats, std::string&& stop) noexcept
{
	stops_.push_back({ std::move(stop), {coordinats.first, coordinats.second} });
	name_stop_.insert({ stops_.back().name, &stops_.back() });
	stop_to_buses_.insert({ &stops_.back(), {} });
}

void transport_catalogue::TransportCatalogue::AddBus(std::string&& name_bus, std::vector<std::string>&& stops_for_bus, bool is_ring, std::string&& name_last_stop) noexcept
{
	Bus temp;
	buses_.push_back(std::move(temp));
	buses_.back().name = std::move(name_bus);
	buses_.back().is_roundtrip = is_ring;
	buses_.back().name_first_stop = stops_for_bus.front();
	buses_.back().name_last_stop = std::move(name_last_stop);

	for (auto it = stops_for_bus.begin(); it != stops_for_bus.end(); ++it)
	{
		buses_.back().stops.push_back(std::move(name_stop_.at(*it)));
		stop_to_buses_[name_stop_.at(*it)].insert(&buses_.back());
	}
	name_bus_.insert({ buses_.back().name, &buses_.back() });
}

void transport_catalogue::TransportCatalogue::AddDistanceBetweenStops(std::string_view nameStop, const std::vector<NearestStop>& distance_to_nearest_stops) noexcept
{
	StopPtr stop1 = FindStop(nameStop);
	for (auto it = distance_to_nearest_stops.begin(); it != distance_to_nearest_stops.end(); ++it)
	{
		StopPtr stop2 = FindStop(it->name_nearest_stop);
		dist_betw_stops_.insert({ {stop1, stop2}, it->distance_to_nearest_stop });
	}
}

std::deque<Bus> transport_catalogue::TransportCatalogue::GetRoute() const noexcept
{
	return buses_;
}

std::deque<Stop> transport_catalogue::TransportCatalogue::GetStop() const noexcept
{
	return stops_;
}

const std::unordered_set<BusPtr>& transport_catalogue::TransportCatalogue::GetBusesByStop(StopPtr stop) const noexcept
{
	static const std::unordered_set<BusPtr> temp;
	auto iter = stop_to_buses_.find(stop);
	return iter == stop_to_buses_.end() ? temp : iter->second;
}

BusStat transport_catalogue::TransportCatalogue::GetStat(transport_catalogue::BusPtr bus) const
{
	BusStat stat;

	std::unordered_set<std::string_view> seen_stops;
	std::optional<geo::Coordinates> prev_pos;

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
		stat.distance += FindDistanceBetweenStops(temp);
	}

	return stat;
}

const std::unordered_map<StopPtr, std::unordered_set<BusPtr>>& transport_catalogue::TransportCatalogue::GetStopToBuses() const noexcept
{
	return stop_to_buses_;
}

size_t transport_catalogue::StopHasher::operator()(const std::pair<StopPtr, StopPtr>& stops) const noexcept
{
	{
		std::hash<std::string>hstr;
		std::hash<double> hdouble;
		return static_cast<size_t>(hstr(stops.first->name))
			+ static_cast<size_t>(hstr(stops.second->name))
			+ static_cast<size_t>(hdouble(stops.first->coordinates.lat))
			+ static_cast<size_t>(hdouble(stops.first->coordinates.lng))
			+ static_cast<size_t>(hdouble(stops.second->coordinates.lat))
			+ static_cast<size_t>(hdouble(stops.second->coordinates.lng));
	}
}

TransportCatalogue* transport_catalogue::TransportCatalogue::Inastance()
{
	if (!tc_)
	{
		tc_ = new TransportCatalogue();
	}
	return tc_;
}
