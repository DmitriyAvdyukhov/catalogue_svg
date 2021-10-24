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
		std::ifstream in("test5.txt");

		if (in)
		{
			RequestReader rr(in);
			transport_catalogue::TransportCatalogue tc;
			add_transport_catologue::AddTransportCatalogueByJson(tc, rr.GetBaseRequest());
			renderer::MapRenderer mr(rr.GetRenderer(), tc);
			RequestHandler rh(tc, mr);
			stat_request::PrintStatDoc(rh, rr.GetStatRequest());
		}
		else
		{
			RequestReader rr(std::cin);
			transport_catalogue::TransportCatalogue tc;
			add_transport_catologue::AddTransportCatalogueByJson(tc, rr.GetBaseRequest());
			renderer::MapRenderer mr(rr.GetRenderer(), tc);
			RequestHandler rh(tc, mr);
			stat_request::PrintStatDoc(rh, rr.GetStatRequest());
		}
	}
	catch (ErrorMassage& e)
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