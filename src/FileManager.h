#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include "Torrent.h"

class FileManager {
private:
    struct FileInfo {
        std::string path;
        int64_t length;
        int64_t offset;
    };

    std::vector<FileInfo> files;
    int64_t total_size;
    int64_t piece_length;

public:
    FileManager(const Torrent& torrent);
    ~FileManager();

    void writePiece(int piece_index, const std::vector<uint8_t>& data);
    
private:
    void prepareFiles();
};
