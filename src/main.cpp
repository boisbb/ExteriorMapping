#include <iostream>

#include "Application.h"

void printUsage()
{
    std::cout << "Usage: " << std::endl << 
                "./ExteriorMapping [ --recover | --config CONFIG_FILE ]" << std::endl << 
                "(CONFIG_FILE needs to be placed in the config file folder in /res)" << std::endl;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> arguments(argv + 1, argv + argc);

    std::string configFile;

    if (arguments.size() == 0)
    {
        printUsage();
        return 0;
    }

    if (arguments[0] == "--config")
    {
        if (arguments.size() == 2)
        {
            configFile = arguments[1];
        }
        else
        {
            printUsage();
        }
    }
    else if (arguments[0] == "--recover")
    {
        configFile = "last.json";
    }
    else
    {
        printUsage();
        return 0;
    }

    if (!std::filesystem::exists(std::string(CONFIG_FILES_LOC) + configFile))
    {
        std::cout << "Error: the config file does not exist." << std::endl;
        return 1;
    }

    auto app = vke::Application(configFile);
    app.run();
}