/**
 * @file Config.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <iostream>
#include <vector>

#include "glm_include_unified.h"
#include "nlohmann/json.hpp"

namespace vke
{
	class ViewGrid;
}

namespace vke::utils
{

struct Config
{
	struct View
	{
		glm::vec3 cameraPos;
		glm::vec3 viewDir;
		uint32_t row;
	};

	bool byStep = false;

	glm::vec3 viewDir;
	glm::vec3 location;
	
	glm::vec2 step;
	glm::ivec2 gridSize;

	bool byInGridPos = false;

	View novelView;
	std::vector<View> views;
	std::string viewGeometry;

	std::vector<std::string> models;
	glm::vec3 lightPos;
};

void saveConfig(std::string configFile, const Config& initConfig, std::shared_ptr<vke::ViewGrid> novelView,
	std::shared_ptr<vke::ViewGrid> viewMatrix);

/**
 * @brief Parses the config file.
 * 
 * @param configFile 
 * @param config 
 */
void parseConfig(std::string configFile, Config& config);

/**
 * @brief Parses only view data.
 * 
 * @param viewData 
 * @param config 
 */
void parseViewData(nlohmann::json viewData, Config& config);

/**
 * @brief Parse config where data is defined by position within
 * 		  the grid.
 * 
 * @param viewData 
 * @param config 
 */
void parseViewsByGridPos(nlohmann::json viewData, Config& config);

/**
 * @brief Parses config where view data is specified by step.
 * 
 * @param viewData 
 * @param config 
 */
void parseViewsByStep(nlohmann::json viewData, Config& config);

/**
 * @brief Parses config where view data is specified by positions 
 * 		  in space.
 * 
 * @param viewData 
 * @param config 
 */
void parseViewsByGrid(nlohmann::json viewData, Config& config);

/**
 * @brief Parses scene data such as models etc.
 * 
 * @param sceneData 
 * @param config 
 */
void parseSceneData(nlohmann::json sceneData, Config& config);

}

