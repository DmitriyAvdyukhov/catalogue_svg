#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <fstream>
#include <string>
#include "domain.h"
using namespace std;
using namespace transport_catalogue;



int main() {

	try
	{
		std::ifstream in("test4.txt");
		std::unique_ptr<RequestReader> rr; 
		std::unique_ptr<renderer::MapRenderer> mr;
		std::unique_ptr<RequestHandler> rh;
		if (in)
		{
			rr = std::make_unique<RequestReader>(in);			
		}
		else
		{
			rr = std::make_unique<RequestReader>(std::cin);			
		}
		transport_catalogue::TransportCatalogue* tc = transport_catalogue::TransportCatalogue::Inastance();
		add_transport_catalogue::AddTransportCatalogueByJson(*tc, rr->GetBaseRequest());
		mr = std::make_unique<renderer::MapRenderer>(rr->GetRenderer(), *tc);
		rh = std::make_unique<RequestHandler>(*tc, *mr);
		stat_request::PrintStatDoc(*rh, rr->GetStatRequest());

	}
	catch (ErrorMessage& e)
	{
		std::cerr << e.what();
	}
	catch (json::ParsingError& e)
	{
		std::cerr << e.what();
	}
	catch (std::logic_error& e)
	{
		std::cerr << e.what();
	}

	return 0;
}