#include "Application.h"
#include "utils/Import.h"
#include "utils/Callbacks.h"
#include "utils/Constants.h"
#include "utils/Structs.h"
#include "utils/Input.h"
#include "utils/FileHandling.h"

// std
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <set>
#include <limits>
#include <algorithm>
#include <unordered_map>

#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

#define PRINT_FPS false


bool enableValidationLayers = true;

using namespace std::chrono;

namespace vke
{

Application::Application()
{
    init();
}

void Application::run()
{
    draw();
}

void Application::init()
{
    m_window = std::make_shared<Window>(WIDTH, HEIGHT);
    m_device = std::make_shared<Device>(m_window);
    m_renderer = std::make_shared<Renderer>(m_device, m_window, "../res/shaders/vert.spv", "../res/shaders/frag.spv");
    m_scene = std::make_shared<Scene>();

    glm::vec3 lightPos = { 0, 100.f, 0 };
    m_scene->setLightPos(lightPos);

    glm::vec2 swapExtent((float)m_renderer->getSwapChain()->getExtent().width,
        (float)m_renderer->getSwapChain()->getExtent().height);

    m_camera = std::make_shared<Camera>(swapExtent, glm::vec3(2.f, 2.f, 2.f));

    //std::shared_ptr<Model> negus = vke::utils::importModel("../res/models/negusPlane/negusPlane.obj",
    //    m_vertices, m_indices);
    //negus->afterImportInit(m_device, m_renderer);

    //std::shared_ptr<Model> pepe = vke::utils::importModel("../res/models/pepePlane/pepePlane.obj",
    //    m_vertices, m_indices);
    //pepe->afterImportInit(m_device, m_renderer);

    //std::shared_ptr<Model> porsche = vke::utils::importModel("../res/models/porsche/porsche.obj",
    //    m_vertices, m_indices);
    //porsche->afterImportInit(m_device, m_renderer);

    std::shared_ptr<Model> sponza = vke::utils::importModel("../res/models/dabrovic_sponza/sponza.obj",
        m_vertices, m_indices);
    sponza->afterImportInit(m_device, m_renderer);

    std::shared_ptr<Model> light = vke::utils::importModel("../res/models/basicCube/cube.obj",
        m_vertices, m_indices);
    light->afterImportInit(m_device, m_renderer);
    glm::mat4 lightMatrix = light->getModelMatrix();
    lightMatrix = glm::scale(lightMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    lightMatrix = glm::translate(lightMatrix, lightPos);
    light->setModelMatrix(lightMatrix);

    m_models.push_back(sponza);
    // m_models.push_back(model2);
    // m_models.push_back(model3);
    // m_models.push_back(light);

    m_scene->setModels(m_device, m_models, m_vertices, m_indices);

    m_renderer->initDescriptorResources();
}

void Application::draw()
{
#if PRINT_FPS
    int frames = 0;
    auto start = high_resolution_clock::now();
#endif

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        vke::utils::consumeDeviceInput(m_window->getWindow(), m_camera);
        m_renderer->renderFrame(m_scene, m_camera);

#if PRINT_FPS
        frames++;
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        int elapsedMs = duration.count();
        float elapsedS = static_cast<float>(elapsedMs) / 1000.f;
        float fps = static_cast<float>(frames) / elapsedS;
        std::cout << fps << std::endl;
#endif

        glfwPollEvents();
    }

    vkDeviceWaitIdle(m_device->getVkDevice());
}

void Application::cleanup()
{
    
}

}