#pragma once
#include <string>

class FileIO {
public:
    // read entire file content as string
    static std::string ReadFile(const std::string& filepath);

    // write string content to file
    static bool WriteFile(const std::string& filepath, const std::string& content);
};