#include "FileIO.h"
#include <fstream>
#include <sstream>

// read text files and write to disk

//e.g. takes in saves/settings and reads everything that is inside and returns what is inside the whole ass file as A STRING
std::string FileIO::ReadFile(const std::string& filepath) {
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open boooo");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

// returns true if writes successfully
bool FileIO::WriteFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath); // creates new or overwrites existing into the file we want

    if (!file.is_open()) {
        return false;
    }

    file << content;
    file.close();

    return true;
}