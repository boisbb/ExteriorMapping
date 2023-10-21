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

bool enableValidationLayers = true;

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

    glm::vec2 swapExtent((float)m_renderer->getSwapChain()->getExtent().width,
        (float)m_renderer->getSwapChain()->getExtent().height);

    m_camera = std::make_shared<Camera>(swapExtent, glm::vec3(2.f, 2.f, 2.f));

    std::shared_ptr<Model> model = vke::utils::importModel("../res/models/basicPlane/plane.obj");
    auto& textureMap = m_renderer->getTextureMap();
    model->afterImportInit(m_device, m_renderer);

    m_models.push_back(model);

    m_renderer->initDescriptorResources();
}

void Application::draw()
{
    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        vke::utils::consumeDeviceInput(m_window->getWindow(), m_camera);
        m_renderer->renderFrame(m_models, m_camera);
        glfwPollEvents();
    }

    vkDeviceWaitIdle(m_device->getVkDevice());
}

void Application::cleanup()
{
    
}

}