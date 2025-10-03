/* ========================================================
 * @file        FileIO.h
 * @author      Tan Ying Clarissa | t.yingclarissa@digipen.edu
 * @date        26 SEP 2025
 *
 * @brief   Header file declaring the FileIO utility class
 *
 * This header file contains the declaration of the FileIO class,
 * which provides functions for file input/output. The class
 * reads entire file contents as strings and write string
 * content to files.
 * 
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>

 /**
  * @class FileIO
  * @brief Utility class providing static file input/output operations.
  *
  * The FileIO class contains static methods for reading from and writing to
  * files. Converts entire file contents to strings and vice versa.
  */
class FileIO {
public:
    /**
     * @brief Reads the entire content of a file as a string.
     *
     * This function opens the specified file and reads its entire contents
     * into a string.
     *
     * @param filepath Path to the file to be read.
     * @return std::string The complete contents of the file as a string.
     *         Returns an empty string if the file cannot be opened or read.
     */
    static std::string ReadFile(const std::string& filepath);

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
    static bool WriteFile(const std::string& filepath, const std::string& content);
};