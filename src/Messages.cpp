#include "Messages.h"
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

Message Message::readFromSocket(int socket_fd) {
    uint32_t length;
    size_t total_len_read = 0;
    while (total_len_read < 4) {
        ssize_t n = recv(socket_fd, reinterpret_cast<char*>(&length) + total_len_read, 4 - total_len_read, 0);
        if (n <= 0) throw std::runtime_error("Socket read error (length)");
        total_len_read += n;
    }
    
    length = ntohl(length);
    if (length > 1024 * 1024) { // 1MB limit for safety
        throw std::runtime_error("Message too large: " + std::to_string(length));
    }
    if (length == 0) { // Keep-alive
        return {0, MessageType::CHOKE, {}}; // Dummy for now
    }

    uint8_t id;
    ssize_t n = recv(socket_fd, &id, 1, 0);
    if (n <= 0) throw std::runtime_error("Socket read error (id)");

    std::vector<uint8_t> payload;
    if (length > 1) {
        payload.resize(length - 1);
        size_t total = 0;
        while (total < length - 1) {
            n = recv(socket_fd, payload.data() + total, length - 1 - total, 0);
            if (n <= 0) throw std::runtime_error("Socket read error (payload)");
            total += n;
        }
    }

    return {length, static_cast<MessageType>(id), std::move(payload)};
}

void Message::writeToSocket(int socket_fd, const Message& msg) {
    uint32_t length_net = htonl(msg.length);
    send(socket_fd, &length_net, 4, 0);
    if (msg.length > 0) {
        uint8_t id = static_cast<uint8_t>(msg.id);
        send(socket_fd, &id, 1, 0);
        if (!msg.payload.empty()) {
            send(socket_fd, msg.payload.data(), msg.payload.size(), 0);
        }
    }
}
