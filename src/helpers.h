#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <ctime>
#include "Torrent.h"

std::string humanReadableSize(int64_t bytes);
std::string humanReadableTime(int64_t seconds);

std::string urlEncode(const uint8_t* data, size_t len);

std::string generateRandomID(size_t length);
std::string generatePeerID();

std::string buildHandshakeMessage(const InfoHash& info_hash, const std::string& client_peer_id);