#include "SceneRoaming.h"

// #define DEBUG
int main()
{
	try
	{
		SceneRoaming app("../data");
		app.run();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "Unknown Error" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}