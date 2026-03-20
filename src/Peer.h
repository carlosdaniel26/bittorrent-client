#include <string>
#include <cstdint>

class Peer {
public:
    Peer(const std::string& ip, uint16_t port) : ip(ip), port(port) {}

    std::string ip;
    uint16_t port;
};