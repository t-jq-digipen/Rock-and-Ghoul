/* ========================================================
 * @file    FileIO.cpp
 * @author  t.yingclarissa@digipen.edu
 * @date    26/09/2025
 *
 * @brief   Implementation file for FileIO utility class
 *
 * This file contains the implementation of the FileIO class,
 * which provides file input/output operations for reading and
 * writing text-based files.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "FileIO.h"
#include <fstream>
#include <sstream>

 /**
  * @brief Reads the entire content of a file as a string.
  *
  * This function opens the specified file using an input file stream and
  * reads its entire contents.
  *
  * @param filepath Path to the file to be read
  * @return std::string Contents of the file as a string.
  * @throws std::runtime_error If the file cannot be opened or read.
  */
std::string FileIO::ReadFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

/**
 * @brief Writes string content to a file.
 *
 * This function creates/overwrites a file at the specified path with
 * the provided string content. If the file already exists, its contents
 * will be replaced. If the file does not exist, it will be created.
 *
 * @param filepath The path to the file to be written.
 * @param content Content to write to the file.
 * @return bool True if the file was successfully written, false if failed
 */
bool FileIO::WriteFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    file.close();
    return true;
}