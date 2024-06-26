/**
 * @file main.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-05-13
 * 
 * 
 */

#include <iostream>

#include "Application.h"

void printUsage()
{
    std::cout << "Usage: " << std::endl << 
                "./ExteriorMapping [ --recover | --config CONFIG_FILE ]" << std::endl << 
                "(CONFIG_FILE needs to be placed in the config file folder in /res)" << std::endl;
}

// Inspired by:
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
            else if (*stringIt == "gt")
            {
                appArgs.evalType = vke::Application::Arguments::EvaluationType::GT;
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
        else if(appArgs.evalType == vke::Application::Arguments::EvaluationType::GT)
        {
            if (auto stringIt = std::next(it, 2); stringIt != arguments.end())
            {
                if (isStringNumber(*stringIt))
                {
                    appArgs.numberOfFrames = std::stoi(*stringIt) + 1;
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

        if (appArgs.evalType == vke::Application::Arguments::EvaluationType::MSE ||
            appArgs.evalType == vke::Application::Arguments::EvaluationType::ONE)
        {
            if (auto stringIt = std::next(it, 3); stringIt != arguments.end())
            {
                if (isStringNumber(*stringIt))
                {
                    appArgs.numberOfSamples = std::stoi(*stringIt);
                }
            }

            if (appArgs.evalType == vke::Application::Arguments::EvaluationType::ONE)
            {
                if (auto stringIt = std::next(it, 4); stringIt != arguments.end())
                {
                    if (isStringNumber(*stringIt))
                    {
                        appArgs.numberOfFrames = std::stoi(*stringIt) + 1;
                    }
                }
            }
        }
        else
        {
            if (auto stringIt = std::next(it, 3); stringIt != arguments.end())
            {
                if (isStringNumber(*stringIt))
                {
                    appArgs.numberOfFrames = std::stoi(*stringIt) + 1;
                }
            }
        }
    }
}

void argumentsWindowSize(const std::vector<std::string>& arguments, vke::Application::Arguments& appArgs)
{
    auto it = arguments.begin();

    appArgs.windowResolution = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    appArgs.novelResolution = glm::vec2(NOVEL_VIEW_WIDTH, NOVEL_VIEW_HEIGHT);
    appArgs.viewGridResolution = glm::vec2(VIEW_MATRIX_WIDTH, VIEW_MATRIX_HEIGHT);

    int i = 0;
    while (i < arguments.size())
    {
        if (arguments[i] == "-w" && (i + 2 + 1 <= arguments.size()))
        {
            if (isStringNumber(arguments[i + 1]) && isStringNumber(arguments[i + 2]))
            {
                appArgs.windowResolution.x = std::stoi(arguments[i + 1]);
                appArgs.windowResolution.y = std::stoi(arguments[i + 2]);
            }
        }

        if (arguments[i] == "-n" && (i + 2 + 1 <= arguments.size()))
        {
            if (isStringNumber(arguments[i + 1]) && isStringNumber(arguments[i + 2]))
            {
                appArgs.novelResolution.x = std::stoi(arguments[i + 1]);
                appArgs.novelResolution.y = std::stoi(arguments[i + 2]);
            }
        }

        if (arguments[i] == "-v" && (i + 2 + 1 <= arguments.size()))
        {
            if (isStringNumber(arguments[i + 1]) && isStringNumber(arguments[i + 2]))
            {
                appArgs.viewGridResolution.x = std::stoi(arguments[i + 1]);
                appArgs.viewGridResolution.y = std::stoi(arguments[i + 2]);
            }
        }

        i++;
    }
}

vke::Application::Arguments parseArguments(const std::vector<std::string>& arguments)
{
    vke::Application::Arguments appArgs{};

    argumentsConfig(arguments, appArgs);
    
    argumentsDebug(arguments, appArgs);


    if (appArgs.evalType == vke::Application::Arguments::EvaluationType::_COUNT)
    {
        argumentsWindowSize(arguments, appArgs);
    }

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