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

#include <vector>
#include <iostream>
#include <fstream>

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

