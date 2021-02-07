#include "Application.hpp"

int main(int argc, char** argv)
{
	try
	{
		gtk::Application app{ argc, argv };
		return app.run();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}
}