#pragma once
#include "request_handler.h"



namespace add_transport_catalogue
{
	struct Bus
	{
		std::string name_bus;
		std::vector<std::string> stops_for_bus;
		bool is_roundtrip = false;

		std::string name_last_stop;
	};

	struct Stop
	{
		std::string name_stop;
		double  latitude = 0.;
		double longitude = 0.;

		std::vector<transport_catalogue::NearestStop> distance_to_nearest_stops;
	};	

	inline Bus ParseBus(const json::Dict& dict);

	inline Stop ParseStop(const json::Dict& dict);

	inline std::vector<Bus> AddBusTemp(const json::Array& temp);

	inline std::vector<Stop> AddStopsTemp(const json::Array& temp);

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

	inline std::vector<StatTemp> StatRequestJsonToVectorRequest(json::Array stat_requests);

	void PrintStatDoc(const RequestHandler& rh, json::Array stat_requests);

}//namespace






