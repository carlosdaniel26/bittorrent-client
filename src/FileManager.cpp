#include "FileManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

FileManager::FileManager(const Torrent& torrent)
    : total_size(torrent.total_size), piece_length(torrent.piece_length)
{
    if (torrent.is_multi_file) {
        int64_t current_offset = 0;
        for (const auto& file : torrent.files) {
            std::string full_path = torrent.name + "/";
            for (size_t i = 0; i < file.path.size(); ++i) {
                full_path += file.path[i];
                if (i < file.path.size() - 1) full_path += "/";
            }
            files.push_back({full_path, file.length, current_offset});
            current_offset += file.length;
        }
    } else {
        files.push_back({torrent.name, torrent.length, 0});
    }

    prepareFiles();
}

FileManager::~FileManager() {}

void FileManager::prepareFiles() {
    for (const auto& file : files) {
        fs::path p(file.path);
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path());
        }
        
        if (!fs::exists(p)) {
            std::ofstream ofs(p, std::ios::binary | std::ios::out);
            ofs.seekp(file.length - 1);
            ofs.put(0);
            ofs.close();
        }
    }
}

void FileManager::writePiece(int piece_index, const std::vector<uint8_t>& data) {
    int64_t piece_offset = (int64_t)piece_index * piece_length;
    int64_t bytes_to_write = data.size();
    int64_t data_pos = 0;

    for (const auto& file : files) {
        if (bytes_to_write <= 0) break;

        // Check if piece falls into this file
        int64_t file_end = file.offset + file.length;
        if (piece_offset < file_end && (piece_offset + bytes_to_write) > file.offset) {
            
            // Calculate starting point in this file
            int64_t start_in_file = std::max((int64_t)0, piece_offset - file.offset);
            // Calculate how much to write to this file
            int64_t end_in_file = std::min(file.length, piece_offset + bytes_to_write - file.offset);
            int64_t write_len = end_in_file - start_in_file;

            std::fstream fs(file.path, std::ios::binary | std::ios::in | std::ios::out);
            if (!fs) {
                std::cerr << "Error opening file for writing: " << file.path << std::endl;
                continue;
            }
            fs.seekp(start_in_file);
            fs.write(reinterpret_cast<const char*>(data.data() + data_pos), write_len);
            fs.close();

            data_pos += write_len;
            bytes_to_write -= write_len;
            piece_offset += write_len;
        }
    }
}
