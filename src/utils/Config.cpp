/**
 * @file Config.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "ViewGrid.h"
#include "utils/Config.h"
#include "utils/Constants.h"

#include <fstream>

namespace vke::utils
{

void saveConfig(std::string configFile, const Config& initConfig, std::shared_ptr<ViewGrid> novelView,
	std::shared_ptr<ViewGrid> viewMatrix)
{
	nlohmann::json j;

	j["viewData"]["geometry"] = initConfig.viewGeometry;

	glm::vec3 novelCameraPos = novelView->getViews()[0]->getCamera()->getEye();
	glm::vec3 novelCameraViewDir = novelView->getViews()[0]->getCamera()->getViewDir();

	j["viewData"]["novelView"]["cameraPos"]["x"] = novelCameraPos.x;
	j["viewData"]["novelView"]["cameraPos"]["y"] = novelCameraPos.y;
	j["viewData"]["novelView"]["cameraPos"]["z"] = novelCameraPos.z;

	j["viewData"]["novelView"]["viewDir"]["x"] = novelCameraViewDir.x;
	j["viewData"]["novelView"]["viewDir"]["y"] = novelCameraViewDir.y;
	j["viewData"]["novelView"]["viewDir"]["z"] = novelCameraViewDir.z;

	j["viewData"]["byStep"] = false;
	j["viewData"]["byInGridPos"] = true;

	std::vector<std::shared_ptr<View>> views = viewMatrix->getViews();
	std::vector<uint32_t> rowCols = viewMatrix->getViewRowsColumns();

	std::vector<nlohmann::json> viewRows;
	int viewId = 0;
	for (int i = 0; i < rowCols.size(); i++)
	{
		std::vector<nlohmann::json> viewRow;
		for (int j = 0; j < rowCols[i]; j++)
		{
			glm::vec3 gridPos = viewMatrix->getViewGridPos(views[viewId]);
			glm::vec3 viewDir = views[viewId]->getCamera()->getViewDir();

			nlohmann::json view;
			view["gridPos"]["x"] = gridPos.x;
			view["gridPos"]["y"] = gridPos.y;
			view["gridPos"]["z"] = gridPos.z;
			
			view["viewDir"]["x"] = viewDir.x;
			view["viewDir"]["y"] = viewDir.y;
			view["viewDir"]["z"] = viewDir.z;

			viewRow.push_back(view);

			viewId++;
		}

		viewRows.push_back(viewRow);
	}

	j["viewData"]["views"] = viewRows;

	glm::vec3 viewGridPos = viewMatrix->getPos();
	glm::vec3 viewGridDir = viewMatrix->getViewDir();

	j["viewData"]["viewGrid"]["pos"]["x"] = viewGridPos.x;
	j["viewData"]["viewGrid"]["pos"]["y"] = viewGridPos.y;
	j["viewData"]["viewGrid"]["pos"]["z"] = viewGridPos.z;

	j["viewData"]["viewGrid"]["viewDir"]["x"] = viewGridDir.x;
	j["viewData"]["viewGrid"]["viewDir"]["y"] = viewGridDir.y;
	j["viewData"]["viewGrid"]["viewDir"]["z"] = viewGridDir.z;

	j["sceneData"]["models"] = initConfig.models;

	j["sceneData"]["lightPos"]["x"] = initConfig.lightPos.x;
	j["sceneData"]["lightPos"]["y"] = initConfig.lightPos.y;
	j["sceneData"]["lightPos"]["z"] = initConfig.lightPos.z;

	std::string jdump = j.dump(4, ' ');

	std::ofstream f(std::string(CONFIG_FILES_LOC) + configFile, std::ios::out);
	f.write(jdump.data(), jdump.length());

	f.close();
}

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
	novelView.viewDir = glm::vec3(
		mainView["viewDir"]["x"].template get<float>(),
		mainView["viewDir"]["y"].template get<float>(),
		mainView["viewDir"]["z"].template get<float>()
	);

	config.novelView = novelView;

	bool byStep = viewData["byStep"].template get<bool>();

	if (byStep)
	{
		parseViewsByStep(viewData, config);
	}
	else
	{
		bool byInGridPos = viewData["byInGridPos"].template get<bool>();
		if (byInGridPos)
			parseViewsByGridPos(viewData, config);
		else	
			parseViewsByGrid(viewData, config);
	}
}

void parseViewsByGridPos(nlohmann::json viewData, Config& config)
{
	config.byInGridPos = true;

	nlohmann::json rows = viewData["views"];

	uint32_t rowCount = 0;
	uint32_t maxCol = 0;
	for (auto& row : rows)
	{
		maxCol = (rows.size() > maxCol) ? rows.size() : maxCol;
		for (auto& col : row)
		{
			Config::View view;
			view.row = rowCount;

			view.cameraPos = glm::vec3(
				col["gridPos"]["x"].template get<float>(),
				col["gridPos"]["y"].template get<float>(),
				col["gridPos"]["z"].template get<float>()
			);

			view.viewDir = glm::vec3(
				col["viewDir"]["x"].template get<float>(),
				col["viewDir"]["y"].template get<float>(),
				col["viewDir"]["z"].template get<float>()
			);

			config.views.push_back(view);
		}
		rowCount++;
	}
	config.gridSize = glm::vec2(maxCol, rowCount);

	config.location = glm::vec3(
		viewData["viewGrid"]["pos"]["x"].template get<float>(),
		viewData["viewGrid"]["pos"]["y"].template get<float>(),
		viewData["viewGrid"]["pos"]["z"].template get<float>()
	);

	config.viewDir = glm::vec3(
		viewData["viewGrid"]["viewDir"]["x"].template get<float>(),
		viewData["viewGrid"]["viewDir"]["y"].template get<float>(),
		viewData["viewGrid"]["viewDir"]["z"].template get<float>()
	);
}

void parseViewsByStep(nlohmann::json viewData, Config &config)
{
	config.byStep = true;

	config.gridSize = glm::ivec2(
		viewData["views"]["gridSize"]["x"].template get<float>(),
		viewData["views"]["gridSize"]["y"].template get<float>()
	);

	config.viewDir = glm::vec3(
		viewData["views"]["viewDir"]["x"].template get<float>(),
		viewData["views"]["viewDir"]["y"].template get<float>(),
		viewData["views"]["viewDir"]["z"].template get<float>()
	);

	config.location = glm::vec3(
		viewData["views"]["location"]["x"].template get<float>(),
		viewData["views"]["location"]["y"].template get<float>(),
		viewData["views"]["location"]["z"].template get<float>()
	);

	config.step = glm::vec2(
		viewData["views"]["step"]["x"].template get<float>(),
		viewData["views"]["step"]["y"].template get<float>()
	);
}

void parseViewsByGrid(nlohmann::json viewData, Config &config)
{
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

void parseSceneData(nlohmann::json sceneData, Config &config)
{
	config.models = std::vector<std::string>();

	std::cout << "huh" << std::endl;

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