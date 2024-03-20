/**
 * @file FileHandling.cpp
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "utils/FileHandling.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

#include <algorithm>

namespace vke::utils
{

std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
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
        throw std::runtime_error("Failed loading image: " + filename);
    }

    return pixels;
}

void saveImage(const std::string &filename, const glm::ivec3 &dims, uint8_t *data)
{
    if (filename.substr(filename.find_last_of(".") + 1) == "jpg")
    {
        stbi_write_jpg("test.jpg",dims.x, dims.y, dims.z, data, 50);
    }
    else if (filename.substr(filename.find_last_of(".") + 1) == "png")
    {
        stbi_write_png("test.png", dims.x, dims.y, dims.z, data, dims.x * dims.z);
    }
    else if (filename.substr(filename.find_last_of(".") + 1) == "ppm")
    {
        std::ofstream file(filename, std::ios::out | std::ios::binary);

        file << "P6\n" << dims.x << "\n" << dims.y << "\n" << 255 << "\n";

        for (uint32_t y = 0; y < dims.y; y++)
        {
            uint32_t *row = (uint32_t*)data;
            for (uint32_t x = 0; x < dims.x; x++)
            {
                file.write((char*)row, 3);
                row++;
            }

            data += (dims.x * dims.z);
        }

        file.close();
    }
    else
    {
        std::cerr << "Error: Unsupported image format." << std::endl;
    }
}

void saveImages(const std::vector<SaveImageInfo> &saveInfos)
{
    for (auto& info : saveInfos)
    {
        saveImage(info.filename, info.dims, info.data);
    }
}

void saveImagesThread(const std::vector<SaveImageInfo> &saveInfos, bool& imagesSaved)
{
    saveImages(saveInfos);
    imagesSaved = true;
}

std::vector<unsigned char> threeChannelsToOne(unsigned char *pixels, const int &width,
                                              const int &height)
{
    std::vector<unsigned char> newValues(width * height);

    for (int i = 0; i < width * height; i++)
    {
        int pixelsId = i * 3;

        newValues[i] = static_cast<float>(pixels[pixelsId] + pixels[pixelsId + 1] + pixels[pixelsId + 2]) / 3.f;
    }

    return newValues;
}

std::vector<unsigned char> threeChannelsToFour(unsigned char* pixels, const int& width,
    const int& height)
{
    std::vector<unsigned char> newValues(width * height * 4);

    for (int i = 0; i < width * height; i++)
    {
        int pixelsId = i * 3;
        int newValsId = i * 4;

        newValues[newValsId] = pixels[pixelsId];
        newValues[newValsId + 1] = pixels[pixelsId + 1];
        newValues[newValsId + 2] = pixels[pixelsId + 2];
        newValues[newValsId + 3] = 255;
    }

    return newValues;
}

}