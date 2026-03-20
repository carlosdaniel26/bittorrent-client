#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstdint>

#include "Peer.h"
#include "Torrent.h"

class Tracker {
public:
    Tracker(const std::string& announce_url);
    std::vector<Peer> getPeers(const InfoHash& info_hash, const std::string&   , int port, int uploaded=0, int downloaded=0, int left=0);

private:
    std::string url;
};