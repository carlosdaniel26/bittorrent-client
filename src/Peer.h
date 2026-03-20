#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <sys/socket.h>
#include <cstdbool>

class Peer {
public:
    Peer(const std::string& ip, uint16_t port) : ip(ip), port(port) {}

    std::string ip;
    uint16_t port;
    int socket_fd;
    bool choke;
    bool interested;
    bool active;
    std::vector<bool> pieces_bitfield;
};