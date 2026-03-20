#include <array>
#include <string>
#include <iostream>

#include "helpers.h"
#include "Torrent.h"


std::string humanReadableSize(int64_t bytes) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    double size = static_cast<double>(bytes);
    int i = 0;
    while (size >= 1024 && i < 5) {
        size /= 1024;
        i++;
    }
    
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << size << " " << suffixes[i];
    return out.str();
}

std::string humanReadableTime(int64_t seconds) {
    std::time_t t = seconds;
    std::tm* tm_info = std::localtime(&t);

    std::ostringstream out;
    out << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

std::string urlEncode(const uint8_t* data, size_t len)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (size_t i = 0; i < len; ++i) {
        char c = data[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }

    return escaped.str();
}

std::string generateRandomID(size_t length)
{
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.resize(length);

    for (size_t i = 0; i < length; ++i) {
        result[i] = charset[rand() % (sizeof(charset) - 1)];
    }

    return result;
}

std::string generatePeerID()
{
    /* 20 bytes total */
    return "-CT1000-" + generateRandomID(12);
}

std::string buildHandshakeMessage(const InfoHash& info_hash, const std::string& client_peer_id)
{
    if (client_peer_id.size() != 20) throw std::runtime_error("client_peer_id must be 20 bytes");
    
    std::string hash_str(info_hash.begin(), info_hash.end());
    

    std::string handshake;
    handshake += char(19); // Protocol string length
    handshake += "BitTorrent protocol"; // Protocol string
    handshake += std::string(8, '\0'); // Reserved bytes
    handshake += hash_str; // Info hash (20 bytes)
    handshake += client_peer_id; // Peer ID (20 bytes)

    if (client_peer_id.size() != 20) throw std::runtime_error("client_peer_id must be 20 bytes");
    return handshake;
}