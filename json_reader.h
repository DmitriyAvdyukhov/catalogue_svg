#pragma once
#include "json.h"

#include <exception>

using namespace std::literals;

class ErrorMessage : public std::exception
{
public:
	ErrorMessage(const std::string& w) : what_(w) {}

	std::string what();

private:
	std::string what_;
};

using NameStop = std::string;
using NameBus = std::string;
using Type = std::string;
using Distance = int;
using Latitude = double;
using Longitude = double;
using StopsForBus = std::vector<NameStop>;
using RoundTrip = bool;

struct NearestStop 
{
	NameStop name;
	Distance distance;
};

struct BaseRecuest
{
	Type type;

	NameStop name_stop;	
	Latitude latitude = 0.;
	Longitude longitude = 0;
	std::vector<::NearestStop> distance_to_nearest_stops;

	NameBus  name_bus;
	StopsForBus stop_for_bus;
	RoundTrip is_roundtrip = false;
};

using Id = int;
using Type = std::string;
using Name = std::string;

struct StatRequest
{
	Id id;
	Type type;
	Name name;
};

class RequestReader
{
public:
	RequestReader() = default;

	explicit RequestReader(std::istream& in);

	json::Array GetBaseRequest()const;

	json::Array GetStatRequest()const;

	json::Dict GetRenderer()const;


private:
	std::vector<BaseRecuest> base_request_;
	std::vector<StatRequest> stat_request_;

	json::Array base_requests_;
	json::Array stat_requests_;
	json::Dict render_settings_;
};

class RequestReader1
{
public:
	RequestReader1() = default;

	RequestReader1(std::istream& in)
	{
		json::Document doc_ = json::Load(in);

		json::Array base_requests = doc_.GetRoot().AsDict().at("base_requests"s).AsArray();

		for (auto it = base_requests.begin(); it != base_requests.end(); ++it)
		{
			if (it->AsDict().at("type"s).AsString() == "Stop"s)
			{
				BaseRecuest temp;
				temp.type = it->AsDict().at("type"s).AsString();
				json::Dict dict = it->AsDict();				
				temp.name_stop = dict.at("name"s).AsString();
				temp.latitude = dict.at("latitude"s).AsDouble();
				temp.longitude = dict.at("longitude"s).AsDouble();
				json::Dict distance_road = dict.at("road_distances"s).AsDict();
				for (auto [name_stop, distance] : distance_road)
				{
					NearestStop ns = { name_stop, distance.AsInt() };
					temp.distance_to_nearest_stops.push_back(std::move(ns));
				}
				
			}
		}


		json::Array stat_requests = doc_.GetRoot().AsDict().at("stat_requests"s).AsArray();

		json::Dict render_settings = doc_.GetRoot().AsDict().at("render_settings"s).AsDict();
	}

	


private:
	std::vector<BaseRecuest> base_request_;
	std::vector<StatRequest> stat_request_;

	
};
