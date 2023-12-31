#include <iostream>
#include <vector>

#include "glm_include_unified.h"
#include "nlohmann/json.hpp"

namespace vke::utils
{

struct Config
{
	struct View
	{
		glm::vec3 cameraPos;
		uint32_t row;
	};

	std::vector<View> views;
	std::string viewGeometry;

	std::vector<std::string> models;
	glm::vec3 lightPos;
};

void parseConfig(std::string configFile, Config& config);

void parseViewData(nlohmann::json viewData, Config& config);

void parseSceneData(nlohmann::json sceneData, Config& config);

}

