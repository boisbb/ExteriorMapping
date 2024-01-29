#include "utils/Config.h"

#include <fstream>

namespace vke::utils
{

void parseConfig(std::string configFile, Config& config)
{
	std::ifstream f(configFile);

	nlohmann::json data = nlohmann::json::parse(f);

	nlohmann::json viewData = data["viewData"];
	parseViewData(viewData, config);

	nlohmann::json sceneData = data["sceneData"];
	parseSceneData(sceneData, config);
}

void parseViewData(nlohmann::json viewData, Config& config)
{
	config.views = std::vector<Config::View>();

	config.viewGeometry = viewData["geometry"].template get<std::string>();
	
	nlohmann::json mainView = viewData["novelView"];
	Config::View novelView;
	novelView.row = 0;
	novelView.cameraPos = glm::vec3(
		mainView["cameraPos"]["x"].template get<float>(),
		mainView["cameraPos"]["y"].template get<float>(),
		mainView["cameraPos"]["z"].template get<float>()
	);

	config.novelView = novelView;

	nlohmann::json rows = viewData["views"];

	uint32_t rowCount = 0;
	for (auto& row : rows)
	{
		for (auto& col : row)
		{
			Config::View view;
			view.row = rowCount;

			view.cameraPos = glm::vec3(
				col["cameraPos"]["x"].template get<float>(),
				col["cameraPos"]["y"].template get<float>(),
				col["cameraPos"]["z"].template get<float>()
			);

			config.views.push_back(view);
		}
		rowCount++;
	}
}

void parseSceneData(nlohmann::json sceneData, Config& config)
{
	config.models = std::vector<std::string>();

	config.lightPos = glm::vec3(
		sceneData["lightPos"]["x"],
		sceneData["lightPos"]["y"],
		sceneData["lightPos"]["z"]
	);

	nlohmann::json models = sceneData["models"];

	for (auto& model : models)
	{
		config.models.push_back(model.template get<std::string>());
	}
}

}