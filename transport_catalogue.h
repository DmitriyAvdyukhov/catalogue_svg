#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <tuple>
#include <deque>
#include <iostream>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <optional>
#include <iomanip>
#include <set>


#include "geo.h"


namespace transport_catalogue
{
	struct Stop
	{
		std::string name;
		geo::Coordinates coordinates{};
	};

	using StopPtr = const transport_catalogue::Stop*;



	struct StopHasher
	{
		size_t operator()(const std::pair<StopPtr, StopPtr>& stops) const noexcept;
	};

	struct Bus
	{
		std::string name;
		std::vector<StopPtr>stops;
		std::vector<std::string> stops_for_bus;
		std::string ending_station_first;
		std::string ending_station_lost;
		bool is_roundtrip = false;
	};

	using BusPtr = const transport_catalogue::Bus*;

	struct StopInfo
	{
		std::string about = "not found";
		std::unordered_set<BusPtr> buses_for_stop;
	};

	struct BusStat
	{
		int total_stops = 0;
		int unique_stops = 0;
		double route_length = 0.;
		int distance = 0;
	};

	struct DistanceStop
	{
		std::string name_stop;
		std::vector<std::pair<std::string, int>> stops_to_stop;
	};
	class TransportCatalogue
	{
	public:

		TransportCatalogue() = default;

		StopPtr FindStop(const std::string_view& stop) const noexcept;

		BusPtr FindBus(const std::string_view& buss) const noexcept;

		int FindDistanceBetweenStops(const std::pair<StopPtr, StopPtr>& stops)const noexcept;

		void AddStop(std::pair<double, double> coordinats, std::string&& stop) noexcept;

		void AddBus(std::string&& name_bus, std::vector<std::string>&& stops_for_bus, bool is_ring, std::string&& station_lost) noexcept;

		void AddDistanceBetweenStops(std::string_view nameStop, std::vector<std::pair< std::string, int>> stops_to_stop) noexcept;

		std::deque<Bus> GetRoute() const noexcept;

		std::deque<Stop> GetStop() const noexcept;

		const std::unordered_set<BusPtr>& GetBusesByStop(StopPtr stop) const noexcept;

		BusStat GetStat(transport_catalogue::BusPtr bus) const;

		const std::unordered_map<StopPtr, std::unordered_set<BusPtr>>& GetStopToBuses()const noexcept;

	private:
		std::deque<Bus>buses_;
		std::deque<Stop>stops_;

		std::unordered_map<StopPtr, std::unordered_set<BusPtr>> stop_to_buses_;

		std::unordered_map<std::string_view, BusPtr> name_bus_;
		std::unordered_map<std::string_view, StopPtr>name_stop_;

		std::unordered_map<std::pair<StopPtr, StopPtr>, int, StopHasher> dist_betw_stops_;
	};
}



