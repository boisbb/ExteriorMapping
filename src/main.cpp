#include <iostream>

#include "Application.h"

void printUsage()
{
    std::cout << "Usage: " << std::endl << 
                "./ExteriorMapping [ --recover | --config CONFIG_FILE ]" << std::endl << 
                "(CONFIG_FILE needs to be placed in the config file folder in /res)" << std::endl;
}

// https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
bool isStringNumber(std::string number)
{
    auto it = number.begin();
    while (it != number.end() && std::isdigit(*it)) ++it;
    return !number.empty() && it == number.end();
}

void argumentsConfig(const std::vector<std::string>& arguments, vke::Application::Arguments& appArgs)
{
    appArgs.configFile = "";
    auto it = arguments.begin();

    if (it = std::find(arguments.begin(), arguments.end(), "--config"); it != arguments.end())
    {  
        if (auto stringIt = std::next(it, 1); stringIt != arguments.end())
            appArgs.configFile = std::string(CONFIG_FILES_LOC) +  *stringIt;
    }
    else if (it = std::find(arguments.begin(), arguments.end(), "--from_snap"); it != arguments.end())
    {
        if (auto stringIt = std::next(it, 1); stringIt != arguments.end())
            appArgs.configFile = std::string(SCREENSHOT_FILES_LOC) + *stringIt;
    }
    else if (it = std::find(arguments.begin(), arguments.end(), "--recover"); it != arguments.end())
    {
        appArgs.configFile = std::string(CONFIG_FILES_LOC) + "last.json";
    }

    if (!std::filesystem::exists(appArgs.configFile))
    {
        std::cout << "Error: the config file: " + appArgs.configFile + " does not exist." << std::endl;
    }
}

void argumentsDebug(const std::vector<std::string>& arguments, vke::Application::Arguments& appArgs)
{
    appArgs.evalType = vke::Application::Arguments::EvaluationType::_COUNT;
    auto it = arguments.begin();
    if (it = std::find(arguments.begin(), arguments.end(), "--eval"); it != arguments.end())
    {  
        if (auto stringIt = std::next(it, 1); stringIt != arguments.end())
        {
            if (*stringIt == "samples")
            {
                appArgs.evalType = vke::Application::Arguments::EvaluationType::SAMPLES;
            }
            else if (*stringIt == "one")
            {
                appArgs.evalType = vke::Application::Arguments::EvaluationType::ONE;
            }
            else if (*stringIt == "mse")
            {
                appArgs.evalType = vke::Application::Arguments::EvaluationType::MSE;
            }
        }

        if (appArgs.evalType == vke::Application::Arguments::EvaluationType::MSE)
        {
            if (auto stringIt = std::next(it, 2); stringIt != arguments.end())
            {
                if (*stringIt == "gt")
                {
                    appArgs.mseGt = true;
                }
            }
        }

        appArgs.samplingType = vke::SamplingType::COLOR;

        if (auto stringIt = std::next(it, 2); stringIt != arguments.end())
        {
            if (*stringIt == "d")
            {
                std::cout << *stringIt << std::endl;
                appArgs.samplingType = vke::SamplingType::DEPTH_DIST;
            }
            else if (*stringIt == "da")
            {
                appArgs.samplingType = vke::SamplingType::DEPTH_ANGLE;
            }
        }

        if (auto stringIt = std::next(it, 3); stringIt != arguments.end())
        {
            if (isStringNumber(*stringIt))
            {
                appArgs.numberOfSamples = std::stoi(*stringIt);
            }
        }
    }
}

void argumentsWindowSize(const std::vector<std::string>& arguments, vke::Application::Arguments& appArgs)
{
    auto it = arguments.begin();

    appArgs.windowResolution = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);

    if (it = std::find(arguments.begin(), arguments.end(), "--window_res"); it != arguments.end())
    {
        if (auto stringIt = std::next(it, 1); stringIt != arguments.end())
        {
            if (isStringNumber(*stringIt))
            {
                appArgs.windowResolution.x = std::stoi(*stringIt);
            }
        }

        if (auto stringIt = std::next(it, 2); stringIt != arguments.end())
        {
            if (isStringNumber(*stringIt))
            {
                appArgs.windowResolution.y = std::stoi(*stringIt);
            }
        }
    }
}

vke::Application::Arguments parseArguments(const std::vector<std::string>& arguments)
{
    vke::Application::Arguments appArgs{};

    argumentsConfig(arguments, appArgs);
    
    argumentsDebug(arguments, appArgs);

    argumentsWindowSize(arguments, appArgs);

    return appArgs;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> arguments(argv + 1, argv + argc);

    vke::Application::Arguments appArgs{};

    appArgs = parseArguments(arguments);
    if (appArgs.configFile.empty()) return 1;

    auto app = vke::Application(appArgs);
    app.run();
}