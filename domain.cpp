#include "domain.h"

using namespace std::literals;
namespace add_transport_catalogue
{
	Bus ParseBus(const json::Dict& dict)
	{
		{
			Bus bus_temp;

			bus_temp.name_bus = dict.at("name"s).AsString();
			bus_temp.is_roundtrip = dict.at("is_roundtrip"s).AsBool();
			json::Array stops = dict.at("stops"s).AsArray();

			for (auto it = stops.begin(); it != stops.end(); ++it)
			{
				bus_temp.stops_for_bus.push_back(it->AsString());
				bus_temp.name_last_stop = bus_temp.stops_for_bus.back();
			}
			if (!bus_temp.is_roundtrip)
			{

				bus_temp.name_last_stop = bus_temp.stops_for_bus.back();
				bus_temp.stops_for_bus.reserve(bus_temp.stops_for_bus.size() * 2 - 1);
				bus_temp.stops_for_bus.insert(bus_temp.stops_for_bus.end(),
					std::next(bus_temp.stops_for_bus.rbegin()), bus_temp.stops_for_bus.rend());
			}

			return bus_temp;
		}
	}

	Stop ParseStop(const json::Dict& dict)
	{
		{
			Stop stop_temp;

			stop_temp.name_stop = dict.at("name"s).AsString();
			stop_temp.latitude = dict.at("latitude"s).AsDouble();
			stop_temp.longitude = dict.at("longitude"s).AsDouble();
			json::Dict distance_road = dict.at("road_distances"s).AsDict();
			for (auto [name_stop, distance] : distance_road)
			{
				transport_catalogue::NearestStop temp = { name_stop, distance.AsInt() };
				stop_temp.distance_to_nearest_stops.push_back(std::move(temp));
			}
			return stop_temp;
		}
	}

	std::vector<Bus> AddBusTemp(const json::Array& temp)
	{
		{
			std::vector<Bus> buses;
			for (auto it = temp.begin(); it != temp.end(); ++it)
			{
				if (it->AsDict().at("type"s).AsString() == "Bus"s)
				{
					buses.push_back(std::move(ParseBus(it->AsDict())));
				}
			}
			return buses;
		}
	}

	std::vector<Stop> AddStopsTemp(const json::Array& temp)
	{
		{
			std::vector<Stop> stops;

			for (auto it = temp.begin(); it != temp.end(); ++it)
			{
				if (it->AsDict().at("type"s).AsString() == "Stop"s)
				{
					stops.push_back(std::move(ParseStop(it->AsDict())));
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
			dist_temp.push_back({ tc.GetStop().back().name, stop.distance_to_nearest_stops });
		}
		for (auto it = dist_temp.begin(); it != dist_temp.end(); ++it)
		{
			tc.AddDistanceBetweenStops(std::move(it->name_stop), std::move(it->distance_to_nearest_stops));
		}
		for (auto bus : AddBusTemp(base_requests))
		{
			tc.AddBus(std::move(bus.name_bus), std::move(bus.stops_for_bus), bus.is_roundtrip, std::move(bus.name_last_stop));
		}
	}

}//namespace add_transport_catologue


namespace stat_request
{
	std::vector<StatTemp> StatRequestJsonToVectorRequest(json::Array stat_requests)
	{
		std::vector<StatTemp> stats;
		for (auto it = stat_requests.begin(); it != stat_requests.end(); ++it)
		{
			if (it->AsDict().at("type").AsString() == "Map")
			{
				stats.push_back({ it->AsDict().at("id").AsInt()
					, it->AsDict().at("type").AsString() });
			}
			else if (it->AsDict().at("type").AsString() == "Stop" || it->AsDict().at("type").AsString() == "Bus")
			{
				stats.push_back({ it->AsDict().at("id").AsInt()
					, it->AsDict().at("type").AsString()
					, it->AsDict().at("name").AsString() });
			}
			else
			{
				std::string file = __FILE__;
				std::string line = std::to_string(__LINE__);
				std::string error = "Incorrect input stat request in file: " + file + " in line: " + line;
				throw ErrorMessage(error);
			}
		}
		return stats;
	}


	void PrintStatDoc(const RequestHandler& rh, json::Array stat_requests)
	{
		json::Array answers;
		for (const auto& stat : StatRequestJsonToVectorRequest(stat_requests))
		{
			if (stat.type == "Bus")
			{
				if (rh.GetBusStat(stat.name_type))
				{
					transport_catalogue::BusStat bus = rh.GetBusStat(stat.name_type).value();

					json::Node node = json::Builder{}
						.StartDict()
						.Key("curvature").Value(bus.distance / bus.route_length)
						.Key("request_id").Value(stat.id_request)
						.Key("route_length").Value(bus.distance)
						.Key("stop_count").Value(bus.total_stops)
						.Key("unique_stop_count").Value(bus.unique_stops)
						.EndDict()
						.Build();

					answers.emplace_back(std::move(node));
				}				
				else
				{
					json::Node node = json::Builder{}
						.StartDict()
						.Key("request_id").Value(stat.id_request)
						.Key("error_message").Value("not found")
						.EndDict()
						.Build();		

					answers.emplace_back(std::move(node));
				}				
			}
			else if (stat.type == "Stop")
			{
				transport_catalogue::StopInfo stop = rh.GetBusesByStop(stat.name_type);
				if (stop.about == "not found")
				{
					json::Node node = json::Builder{}
						.StartDict()
						.Key("request_id").Value(stat.id_request)
						.Key("error_message").Value("not found")
						.EndDict()
						.Build();

					answers.emplace_back(std::move(node));
				}
				else
				{					
					std::set<std::string>name_bus;
					for (auto buses : stop.buses_for_stop)
					{
						name_bus.insert(buses->name);

					}

					json::Builder answer_stop;
					answer_stop.StartDict()
						.Key("buses").StartArray();

					for (const auto& name : name_bus)
					{
						answer_stop.Value(name);
					}

					answer_stop.EndArray()
						.Key("request_id").Value(stat.id_request)
						.EndDict();	

					answers.emplace_back(std::move(answer_stop.Build()));
				}				
			}
			else if (stat.type == "Map")
			{
				json::Node node = json::Builder{}.StartDict()
					.Key("map").Value(std::move(*rh.GetMap()))
					.Key("request_id").Value(stat.id_request).EndDict().Build();
				answers.emplace_back(std::move(node));				
			}
			else
			{
				std::string file = __FILE__;
				std::string line = std::to_string(__LINE__);
				std::string error = "Incorrect input stat request in file: " + file + " in line: " + line;
				throw ErrorMessage(error);
			}
		}

		json::Node node = json::Node(answers);
		json::Document doc(node);
		json::Print(doc, std::cout);
	}

}//namespace stat_request
