#include "Tracker.h"
#include "Bencode.h"
#include "Torrent.h"

#include <curl/curl.h>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <stdexcept>
#include "Bencode.h"
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <arpa/inet.h>
#include <iostream>

#include "helpers.h"

std::string urlEncodeInfoHash(const InfoHash& hash) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : hash)
        oss << '%' << std::setw(2) << static_cast<int>(c);
    return oss.str();
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), size*nmemb);
    return size*nmemb;
}

std::vector<Peer> Tracker::getPeers(    
    const InfoHash& info_hash,
    const std::string& peer_id,
    int port,
    int uploaded,
    int downloaded,
    int left
) {
    Bencode bencode;

    std::vector<Peer> peersList;

    std::string encoded_peer_id = urlEncode(reinterpret_cast<const uint8_t*>(peer_id.data()), peer_id.size());
    std::cout << "Peer ID: " << peer_id << " Encoded: " << encoded_peer_id << std::endl;

    std::string url_full = url + "?info_hash=" + urlEncodeInfoHash(info_hash) +
                           "&peer_id=" + encoded_peer_id +
                           "&port=" + std::to_string(port) +
                           "&uploaded=" + std::to_string(uploaded) +
                           "&downloaded=" + std::to_string(downloaded) +
                           "&left=" + std::to_string(left) +
                           "&compact=1&event=started";

    std::cout << "Requesting URL: " << url_full << std::endl;

    CURL* curl = curl_easy_init();
    std::string response;
    if (!curl)
        throw std::runtime_error("Cannot init curl");

    curl_easy_setopt(curl, CURLOPT_URL, url_full.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "GeminiBittorrent/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("Tracker request failed: ") + curl_easy_strerror(res));

    std::cout << "Tracker response received, length: " << response.length() << std::endl;

    // parse response com Bencode
    size_t pos = 0;
    Bencode::Value root = bencode.parseValue(response, pos);
    std::cout << "Bencode parse completed" << std::endl;
    if (root.type != Bencode::Value::Type::Dictionary)
        throw std::runtime_error("Tracker response is not a dictionary");

    auto& dict = root.dict_value;
    if (!dict.count("peers"))
        throw std::runtime_error("No peers in tracker response");

    std::string peers_binary = dict["peers"].string_value;

    for (size_t i = 0; i+6 <= peers_binary.size(); i += 6) {
        uint8_t ip1 = static_cast<uint8_t>(peers_binary[i]);
        uint8_t ip2 = static_cast<uint8_t>(peers_binary[i+1]);
        uint8_t ip3 = static_cast<uint8_t>(peers_binary[i+2]);
        uint8_t ip4 = static_cast<uint8_t>(peers_binary[i+3]);
        uint16_t p = (static_cast<uint8_t>(peers_binary[i+4]) << 8) |
                      static_cast<uint8_t>(peers_binary[i+5]);
        std::string ip = std::to_string(ip1) + "." +
                         std::to_string(ip2) + "." +
                         std::to_string(ip3) + "." +
                         std::to_string(ip4);
        peersList.emplace_back(ip, p);
    }

    return peersList;
}

Tracker::Tracker(const std::string& announce_url) : url(announce_url) {}
