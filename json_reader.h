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


