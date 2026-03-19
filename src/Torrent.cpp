#include "Torrent.h"
#include "Bencode.h"
#include "helpers.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <ctime>

Torrent Torrent::fromFile(const std::string& path)
{
    Bencode bencode;
    Torrent t;
    std::ifstream file (path, std::ios::in | std::ios::binary);

    if (!file)
        throw std::runtime_error("Failed to open torrent file: " + path);

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();

    size_t pos = 0;
    Bencode::Value root = bencode.parseValue(data, pos);

    if (root.type != Bencode::Value::Type::Dictionary)
        throw std::runtime_error("Torrent file root is not a dictionary");

    auto& dict = root.dict_value;

    if (dict.count("announce"))
        t.announce = dict["announce"].string_value;

    if (dict.count("comment"))
        t.comment = dict["comment"].string_value;

    if (dict.count("creation date"))
        t.creation_date = dict["creation date"].int_value;

    if (!dict.count("info"))
        throw std::runtime_error("Missing 'info' dictionary");

    auto& info = dict["info"];
    t.raw_info = bencode.serialize(info);

    if (info.dict_value.count("piece length"))
        t.piece_length = info.dict_value["piece length"].int_value;

    if (info.dict_value.count("name"))
        t.name = info.dict_value["name"].string_value;

    if (info.dict_value.count("length"))
        t.length = info.dict_value["length"].int_value;

    if (info.dict_value.count("files")) // multi-file
    {
        t.files.clear();
        for (auto& val : info.dict_value["files"].list_value) {
            Torrent::File f;

            // Build full path from Bencode path array
            f.path.clear();
            for (auto& part : val.dict_value["path"].list_value) {
                f.path.push_back(part.string_value);
            }

            f.length = val.dict_value["length"].int_value;

            t.files.push_back(f);
        }

        t.is_multi_file = true;
    }

    return t;
}

void Torrent::printInfo() const
{   

    std::cout << "Announce: " << announce << std::endl;
    std::cout << "Comment: " << comment << std::endl;
    std::cout << "Creation Date: " << humanReadableTime(creation_date) << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Piece Length: " << humanReadableSize(piece_length) << std::endl;
    std::cout << "Length: " << humanReadableSize(length) << std::endl;
    if (is_multi_file) {
        std::cout << "Files:" << std::endl;
        for (const auto& file : files) {
            std::cout << "  - Length: " << humanReadableSize(file.length) << ", Path: ";
            for (const auto& part : file.path) {
                std::cout << part << "/";
            }
            std::cout << std::endl;
        }
    }
}