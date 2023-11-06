#include "utils/FileHandling.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

#include <algorithm>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vke::utils
{

std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::cout << filename << std::endl;
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

unsigned char* loadImage(std::string& filename, int& width, int& height, int& channels)
{
    std::replace(filename.begin(), filename.end(), '\\', '/');

    stbi_set_flip_vertically_on_load(1);
    unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (pixels == NULL)
    {
        std::cout << filename << std::endl;
        throw std::runtime_error("Failed loading image.");
    }

    return pixels;
}

std::vector<unsigned char> threeChannelsToOne(unsigned char* pixels, const int& width,
    const int& height)
{
    std::vector<unsigned char> newValues(width * height);

    for (int i = 0; i < width * height; i++)
    {
        int pixelsId = i * 3;

        newValues[i] = static_cast<float>(pixels[pixelsId] + pixels[pixelsId + 1] + pixels[pixelsId + 2]) / 3.f;
    }

    return newValues;
}

}