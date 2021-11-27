#pragma once
#include "json_reader.h"
#include "map_renderer.h"

#include <optional>





class RequestHandler
{
public:

    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    std::optional<transport_catalogue::BusStat> GetBusStat(const std::string_view& bus_name) const;

    transport_catalogue::StopInfo GetBusesByStop(const std::string_view& stop_name) const;

    std::optional<std::string> GetMap()const;

private:
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};

