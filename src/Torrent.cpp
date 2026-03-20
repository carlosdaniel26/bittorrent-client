#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <openssl/sha.h>
#include <cstring>
#include <optional>

#include "Torrent.h"
#include "Tracker.h"
#include "Bencode.h"
#include "helpers.h"

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

    if (dict.count("announce")) {
        t.announce = dict["announce"].string_value;
        if (t.trackers.empty()) {
            t.trackers.push_back({t.announce});
        }
    }
    
    if (dict.count("announce-list")) {
        auto& announce_list = dict["announce-list"].list_value;

        for (auto& tier : announce_list) {
            std::vector<std::string> tier_list;

            for (auto& url : tier.list_value) {
                tier_list.push_back(url.string_value);
            }
            
        }

        /* populate trackers*/
        for (const auto& tier : announce_list) {
            std::vector<std::string> tier_list;

            for (const auto& url : tier.list_value) {
                tier_list.push_back(url.string_value);
            }

            t.trackers.push_back(tier_list);
        }
    }

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
        t.total_size = 0;
        for (auto& val : info.dict_value["files"].list_value) {
            Torrent::File f;

            // Build full path from Bencode path array
            f.path.clear();
            for (auto& part : val.dict_value["path"].list_value) {
                f.path.push_back(part.string_value);
            }

            f.length = val.dict_value["length"].int_value;
            t.total_size += f.length;

            t.files.push_back(f);
        }

        t.is_multi_file = true;
    } else {
        t.total_size = t.length;
    }
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(t.raw_info.data()), t.raw_info.size(), hash);
    std::memcpy(t.info_hash.data(), hash, 20);

    /* parse pieces hashs */
    if (info.dict_value.count("pieces")) {
        std::string pieces_str = info.dict_value["pieces"].string_value;
        size_t num_pieces = pieces_str.size() / 20;
        t.piece_count = num_pieces;
        t.pieces.resize(num_pieces);
        for (size_t i = 0; i < num_pieces; ++i) {
            std::memcpy(t.pieces[i].data(), pieces_str.data() + i * 20, 20);
        }
    }

    return t;
}

void Torrent::printInfo() const
{   
    std::cout << "Announce: " << announce << std::endl;
    std::cout << "Comment: " << comment << std::endl;
    std::cout << "Creation Date: " << humanReadableTime(creation_date) << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Info Hash: " << urlEncode(info_hash.data(), info_hash.size()) << std::endl;
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

std::optional<std::string> Torrent::selectWorkingTracker(const std::string& peer_id) {
    std::cout << "Selecting working tracker..." << std::endl;
    for (const auto& tier : trackers) {
        for (const auto& url : tier) {
            try {
                std::cout << "Testing tracker: " << url << std::endl;
                Tracker tracker(url);

                if (tracker.getPeers(info_hash, peer_id, 6881).size() > 0) {
                    return url;
                }

            } catch (const std::exception& e) {
                std::cerr << "Tracker failed: " << e.what() << std::endl;
            }
        }
    }
    return std::nullopt;
}