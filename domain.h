#pragma once
#include "request_handler.h"



namespace add_transport_catologue
{
	struct Buses
	{
		std::string name_bus;
		std::vector<std::string> stops_for_bus;
		bool is_roundtrip = false;
		std::string ending_station_lost;
	};

	struct Stops
	{
		std::string name_stop;
		double  latitude = 0.;
		double longitude = 0.;
		std::vector<std::pair<std::string, int>> stops_to_stop;
	};

	struct DistanceStop
	{
		std::string name_stop;
		std::vector<std::pair<std::string, int>> stops_to_stop;
	};

	inline Buses ParsingBus(const json::Dict& dict);

	inline Stops ParsingStop(const json::Dict& dict);

	inline std::vector<Buses> AddBusTemp(const json::Array& temp);

	inline std::vector<Stops>AddStopsTemp(const json::Array& temp);

	void AddTransportCatalogueByJson(transport_catalogue::TransportCatalogue& tc, json::Array base_requests);

}//namespace


namespace stat_request
{
	struct StatTemp
	{
		int id_request{};
		std::string type{};
		std::string name_type{};
	};

	inline std::vector<StatTemp>AddStatRequestVector(json::Array stat_requests);

	void PrintStatDoc(const RequestHandler& rh, json::Array stat_requests);

}//namespace






