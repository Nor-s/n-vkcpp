#include "class/application.h"
#include <cstdlib>
#include <ctime>

int main()
{
    srand(static_cast<unsigned int>(time(NULL)));
    painting::PaintingApplication app;

    try
    {
        app.run(1024, 512);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}