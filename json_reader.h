#pragma once
#include "json.h"

#include <exception>



class ErrorMassage : public std::exception
{
public:
	ErrorMassage(const std::string& w) : what_(w) {}

	std::string what();

private:
	std::string what_;
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

	json::Array base_requests_;
	json::Array stat_requests_;
	json::Dict render_settings_;
};

