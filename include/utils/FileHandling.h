#pragma once

#include <vector>
#include <iostream>
#include <fstream>

namespace vke::utils
{

std::vector<char> readFile(const std::string& filename);
unsigned char* loadImage(std::string& filename, int& width, int& height, int& channels);

}

