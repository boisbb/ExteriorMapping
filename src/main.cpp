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
            configFile = std::string(CONFIG_FILES_LOC) + arguments[1];
        }
        else
        {
            printUsage();
        }
    }
    else if (arguments[0] == "--fromSnap")
    {
        if (arguments.size() == 2)
        {
            configFile = std::string(SCREENSHOT_FILES_LOC) + arguments[1];
        }
        else
        {
            printUsage();
        }
    }
    else if (arguments[0] == "--recover")
    {
        configFile = std::string(CONFIG_FILES_LOC) + "last.json";
    }
    else
    {
        printUsage();
        return 0;
    }

    if (!std::filesystem::exists(configFile))
    {
        std::cout << "Error: the config file: " + configFile + " does not exist." << std::endl;
        return 1;
    }

    auto app = vke::Application(configFile);
    app.run();
}