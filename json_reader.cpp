#include "json_reader.h"

using namespace std::literals;


std::string ErrorMessage::what()
{
	return what_;
}

RequestReader::RequestReader(std::istream& in)
{
	json::Document doc = json::Load(in);
	base_requests_ = doc.GetRoot().AsDict().at("base_requests"s).AsArray();
	stat_requests_ = doc.GetRoot().AsDict().at("stat_requests"s).AsArray();
	render_settings_ = doc.GetRoot().AsDict().at("render_settings"s).AsDict();
}

json::Array RequestReader::GetBaseRequest()const
{
	return base_requests_;
}

json::Array RequestReader::GetStatRequest()const
{
	return stat_requests_;
}

json::Dict RequestReader::GetRenderer()const
{
	return render_settings_;
}
