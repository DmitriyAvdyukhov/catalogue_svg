#include "domain.h"


namespace add_transport_catologue
{
	Buses ParsingBus(const json::Dict& dict)
	{
		{
			Buses bus_temp;

			bus_temp.name_bus = dict.at("name"s).AsString();
			bus_temp.is_roundtrip = dict.at("is_roundtrip"s).AsBool();
			json::Array stops = dict.at("stops"s).AsArray();

			for (auto it = stops.begin(); it != stops.end(); ++it)
			{
				bus_temp.stops_for_bus.push_back(it->AsString());
				bus_temp.ending_station_lost = bus_temp.stops_for_bus.back();
			}
			if (!bus_temp.is_roundtrip)
			{

				bus_temp.ending_station_lost = bus_temp.stops_for_bus.back();
				bus_temp.stops_for_bus.reserve(bus_temp.stops_for_bus.size() * 2 - 1);
				bus_temp.stops_for_bus.insert(bus_temp.stops_for_bus.end(),
					std::next(bus_temp.stops_for_bus.rbegin()), bus_temp.stops_for_bus.rend());
			}

			return bus_temp;
		}
	}

	Stops ParsingStop(const json::Dict& dict)
	{
		{
			Stops stop_temp;

			stop_temp.name_stop = dict.at("name"s).AsString();
			stop_temp.latitude = dict.at("latitude"s).AsDouble();
			stop_temp.longitude = dict.at("longitude"s).AsDouble();
			json::Dict distance_road = dict.at("road_distances"s).AsMap();
			for (auto [name_stop, distance] : distance_road)
			{
				std::pair<std::string, int> temp = { name_stop, distance.AsInt() };
				stop_temp.stops_to_stop.push_back(std::move(temp));
			}
			return stop_temp;
		}
	}

	std::vector<Buses> AddBusTemp(const json::Array& temp)
	{
		{
			std::vector<Buses> buses;
			for (auto it = temp.begin(); it != temp.end(); ++it)
			{
				if (it->AsMap().at("type"s).AsString() == "Bus"s)
				{
					buses.push_back(std::move(ParsingBus(it->AsMap())));
				}
			}
			return buses;
		}
	}

	std::vector<Stops> AddStopsTemp(const json::Array& temp)
	{
		{
			std::vector<Stops> stops;

			for (auto it = temp.begin(); it != temp.end(); ++it)
			{
				if (it->AsMap().at("type"s).AsString() == "Stop"s)
				{
					stops.push_back(std::move(ParsingStop(it->AsMap())));
				}
			}
			return stops;
		}
	}

	void AddTransportCatalogueByJson(transport_catalogue::TransportCatalogue& tc, json::Array base_requests)
	{
		std::vector<transport_catalogue::DistanceStop> dist_temp;
		for (auto stop : AddStopsTemp(base_requests))
		{
			tc.AddStop({ stop.latitude, stop.longitude }, std::move(stop.name_stop));
			dist_temp.push_back({ tc.GetStop().back().name, stop.stops_to_stop });
		}
		for (auto it = dist_temp.begin(); it != dist_temp.end(); ++it)
		{
			tc.AddDistanceBetweenStops(std::move(it->name_stop), std::move(it->stops_to_stop));
		}
		for (auto bus : AddBusTemp(base_requests))
		{
			tc.AddBus(std::move(bus.name_bus), std::move(bus.stops_for_bus), bus.is_roundtrip, std::move(bus.ending_station_lost));
		}
	}

}//namespace add_transport_catologue


namespace stat_request
{
	std::vector<StatTemp> AddStatRequestVector(json::Array stat_requests)
	{
		std::vector<StatTemp>stats;
		for (auto it = stat_requests.begin(); it != stat_requests.end(); ++it)
		{
			if (it->AsMap().at("type").AsString() == "Map")
			{
				stats.push_back({ it->AsMap().at("id").AsInt()
					, it->AsMap().at("type").AsString() });
			}
			else if (it->AsMap().at("type").AsString() == "Stop" || it->AsMap().at("type").AsString() == "Bus")
			{
				stats.push_back({ it->AsMap().at("id").AsInt()
					, it->AsMap().at("type").AsString()
					, it->AsMap().at("name").AsString() });
			}
			else
			{
				std::string file = __FILE__;
				std::string line = std::to_string(__LINE__);
				std::string error = "Incorrect input stat request in file: " + file + " in line: " + line;
				throw ErrorMassage(error);
			}
		}
		return stats;
	}


	void PrintStatDoc(const RequestHandler& rh, json::Array stat_requests)
	{
		json::Array answers;
		for (const auto& stat : AddStatRequestVector(stat_requests))
		{
			if (stat.type == "Bus")
			{
				json::Dict dict;
				if (rh.GetBusStat(stat.name_type))
				{
					transport_catalogue::BusStat bus = rh.GetBusStat(stat.name_type).value();
					dict["curvature"] = json::Node(bus.distance / bus.route_length);
					dict["request_id"] = json::Node(stat.id_request);
					dict["route_length"] = json::Node(bus.distance);
					dict["stop_count"] = json::Node(bus.total_stops);
					dict["unique_stop_count"] = json::Node(bus.unique_stops);
				}
				else
				{
					dict["request_id"] = json::Node(stat.id_request);
					dict["error_message"] = json::Node(std::string("not found"));
				}

				answers.emplace_back(std::move(json::Node(dict)));
			}
			else if (stat.type == "Stop")
			{
				json::Dict dict_stop;


				transport_catalogue::StopInfo stop = rh.GetBusesByStop(stat.name_type);
				if (stop.about == "not found")
				{
					dict_stop["request_id"] = json::Node(stat.id_request);
					dict_stop["error_message"] = json::Node(std::string("not found"));
				}
				else
				{
					json::Array ar;
					std::set<std::string>name_bus;
					for (auto buses : stop.buses_for_stop)
					{
						name_bus.insert(buses->name);

					}
					for (const auto& name : name_bus)
					{
						ar.push_back(json::Node(std::string(name)));
					}
					dict_stop["buses"] = json::Node(ar);
					dict_stop["request_id"] = json::Node(stat.id_request);
				}
				answers.emplace_back(std::move(json::Node(dict_stop)));
			}
			else if (stat.type == "Map")
			{
				json::Dict dict_map;
				dict_map["map"] = json::Node(std::string(std::move(*rh.GetMap())));
				dict_map["request_id"] = json::Node(stat.id_request);
				answers.emplace_back(std::move(json::Node(dict_map)));
			}
			else
			{
				std::string file = __FILE__;
				std::string line = std::to_string(__LINE__);
				std::string error = "Incorrect input stat request in file: " + file + " in line: " + line;
				throw ErrorMassage(error);
			}
		}

		json::Node node = json::Node(answers);
		json::Document doc(node);
		json::Print(doc, std::cout);
	}

}//namespace stat_request
