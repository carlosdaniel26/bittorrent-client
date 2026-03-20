#pragma once

#include <cstdint>
#include <vector>
#include <arpa/inet.h>
#include <string>

enum class MessageType : uint8_t {
    CHOKE = 0,
    UNCHOKE = 1,
    INTERESTED = 2,
    NOT_INTERESTED = 3,
    HAVE = 4,
    BITFIELD = 5,
    REQUEST = 6,
    PIECE = 7,
    CANCEL = 8,
    PORT = 9
};

struct Message {
    uint32_t length;
    MessageType id;
    std::vector<uint8_t> payload;

    static Message readFromSocket(int socket_fd);
    static void writeToSocket(int socket_fd, const Message& msg);
};
