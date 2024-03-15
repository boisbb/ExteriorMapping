/**
 * @file FileHandling.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "glm_include_unified.h"

#include <vector>
#include <iostream>
#include <fstream>

#include "Structs.h"

namespace vke::utils
{

/**
 * @brief Reads a file into a vector of chars.
 * 
 * @param filename 
 * @return std::vector<char> 
 */
std::vector<char> readFile(const std::string& filename);

/**
 * @brief Loads an image using the stb image library
 * 
 * @param filename 
 * @param width 
 * @param height 
 * @param channels 
 * @return unsigned* 
 */
unsigned char* loadImage(std::string& filename, int& width, int& height, int& channels);

/**
 * @brief Saves image using the stb image library.
 * 
 * @param filename 
 * @param dims W * H data, z coordinate is the number of channels.
 * @param data 
 */
void saveImage(const std::string& filename, const glm::ivec3& dims, uint8_t* data);

/**
 * @brief Saves multiple images in one go.
 * 
 * @param saveInfos 
 */
void saveImages(const std::vector<SaveImageInfo>& saveInfos);

/**
 * @brief Save images and notify when done.
 * 
 * @param saveInfos 
 * @param imagesSaved 
 */
void saveImagesThread(const std::vector<SaveImageInfo> &saveInfos, bool& imagesSaved);

/**
 * @brief Transform image of three channels to one channel image.
 * 
 * @param pixels 
 * @param width 
 * @param height 
 * @return std::vector<unsigned char> 
 */
std::vector<unsigned char> threeChannelsToOne(unsigned char* pixels, const int& width,
    const int& height);

/**
 * @brief Transform image of three channels to four channel image.
 * 
 * @param pixels 
 * @param width 
 * @param height 
 * @return std::vector<unsigned char> 
 */
std::vector<unsigned char> threeChannelsToFour(unsigned char* pixels, const int& width,
    const int& height);

}

