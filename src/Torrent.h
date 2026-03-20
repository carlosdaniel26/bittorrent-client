#pragma once 

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <array>

#include "Bencode.h"

using InfoHash = std::array<unsigned char, 20>;

class Torrent {
 public:
    static Torrent fromFile(const std::string& path);
    static Torrent fromBytes(const std::string& data);
    void printInfo() const;

    std::string announce;
    std::vector<std::string> announce_list;
    InfoHash info_hash;

    std::string comment;
    int64_t creation_date;

    std::string name;
    int64_t piece_length;

    std::vector<InfoHash> pieces;

    /* actual files */
    struct File {
        int64_t length;
        std::vector<std::string> path;
    };

    bool is_multi_file = false;
    int64_t length = 0;
    std::vector<File> files;

    int64_t total_size = 0;
    int piece_count = 0;

    std::string raw_info;

    Torrent() = default;


};