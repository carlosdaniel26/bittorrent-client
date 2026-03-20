#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <stdexcept>
#include <cstring>
#include <iostream>

#include "PeerManager.h"
#include "helpers.h"

void PeerManager::addPeer(const Peer& peer)
{
    // std::cout << "Adding peer " << peer.ip << " to manager" << std::endl;
    peers.push_back(peer);
}

void PeerManager::addPeers(const std::vector<Peer>& new_peers)
{
    peers.insert(peers.end(), new_peers.begin(), new_peers.end());
}

bool PeerManager::connectToPeer(Peer& peer, int timeout_sec)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;

    // set non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(peer.port);
    if (inet_pton(AF_INET, peer.ip.c_str(), &addr.sin_addr) <= 0) {
        close(sockfd);
        return false;
    }

    int res = connect(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (res < 0 && errno != EINPROGRESS) {
        close(sockfd);
        return false;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);
    timeval tv{timeout_sec, 0};

    res = select(sockfd + 1, nullptr, &wfds, nullptr, &tv);
    if (res <= 0) { // timeout or error
        close(sockfd);
        return false;
    }

    int err = 0;
    socklen_t len = sizeof(err);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (err != 0) {
        close(sockfd);
        return false;
    }

    fcntl(sockfd, F_SETFL, flags);

    peer.socket_fd = sockfd;
    peer.active = true;
    return true;
}

bool PeerManager::sendHandshake(Peer& peer, const std::string& client_peer_id)
{
    std::string handshake = buildHandshakeMessage(info_hash, client_peer_id);
    size_t total_sent = 0;
    while (total_sent < handshake.size()) {
        ssize_t sent = send(peer.socket_fd, handshake.data() + total_sent, handshake.size() - total_sent, 0);
        if (sent <= 0) return false;
        total_sent += sent;
    }
    return true;
}

bool PeerManager::receiveHandshake(Peer& peer, int timeout_sec)
{
    char buffer[68];
    size_t total = 0;

    struct timeval tv{timeout_sec, 0};
    setsockopt(peer.socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    while (total < sizeof(buffer)) {
        ssize_t n = recv(peer.socket_fd, buffer + total, sizeof(buffer) - total, 0);
        if (n <= 0) {
            return false;
        }
        total += n;
    }

    if (static_cast<uint8_t>(buffer[0]) != 19) {
        return false;
    }

    if (std::string(buffer + 1, 19) != "BitTorrent protocol") {
        return false;
    }

    if (std::memcmp(buffer + 28, info_hash.data(), 20) != 0) {
        return false;
    }

    return true;
}

void PeerManager::processPeerMessage(Peer& peer) {
    try {
        Message msg = Message::readFromSocket(peer.socket_fd);
        switch (msg.id) {
            case MessageType::CHOKE:
                peer.choke = true;
                break;
            case MessageType::UNCHOKE:
                peer.choke = false;
                break;
            case MessageType::INTERESTED:
                // Peer is interested in us
                break;
            case MessageType::BITFIELD: {
                peer.pieces_bitfield.clear();
                for (uint8_t byte : msg.payload) {
                    for (int i = 7; i >= 0; --i) {
                        peer.pieces_bitfield.push_back((byte >> i) & 1);
                    }
                }
                // Bitfield might have extra bits at the end, trim to match actual piece count
                if (piece_manager && peer.pieces_bitfield.size() > (size_t)piece_manager->getTotalPieces()) {
                    peer.pieces_bitfield.resize(piece_manager->getTotalPieces());
                }
                break;
            }
            case MessageType::HAVE: {
                if (msg.payload.size() == 4) {
                    uint32_t index_net;
                    std::memcpy(&index_net, msg.payload.data(), 4);
                    uint32_t index = ntohl(index_net);
                    if (!peer.pieces_bitfield.empty() && index < peer.pieces_bitfield.size()) {
                        peer.pieces_bitfield[index] = true;
                    }
                }
                break;
            }
            case MessageType::PIECE: {
                if (msg.payload.size() >= 8) {
                    uint32_t index_net, begin_net;
                    std::memcpy(&index_net, msg.payload.data(), 4);
                    std::memcpy(&begin_net, msg.payload.data() + 4, 4);
                    uint32_t index = ntohl(index_net);
                    uint32_t begin = ntohl(begin_net);
                    std::vector<uint8_t> block_data(msg.payload.begin() + 8, msg.payload.end());
                    
                    if (piece_manager) {
                        piece_manager->saveBlock(index, begin, block_data);
                    }
                }
                break;
            }
            default:
                break;
        }
    } catch (const std::exception& e) {
        // std::cerr << "Error reading message from " << peer.ip << ": " << e.what() << std::endl;
        peer.active = false;
        close(peer.socket_fd);
        peer.socket_fd = -1;
    }
}

void PeerManager::startConnections() {
    std::cout << "Starting connections loop with " << peers.size() << " peers" << std::endl;
    // Non-blocking message loop using select
    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = -1;

        for (auto& peer : peers) {
            if (peer.active && peer.socket_fd != -1) {
                FD_SET(peer.socket_fd, &read_fds);
                if (peer.socket_fd > max_fd) max_fd = peer.socket_fd;
            }
        }

        if (max_fd == -1) {
            std::cout << "\nNo active peers, exiting message loop" << std::endl;
            break;
        }

        struct timeval tv{1, 0}; // 1 second timeout
        int res = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);

        if (res < 0) {
            if (errno == EINTR) continue;
            std::cerr << "Select error: " << strerror(errno) << std::endl;
            break;
        }

        if (res > 0) {
            for (auto& peer : peers) {
                if (peer.active && peer.socket_fd != -1 && FD_ISSET(peer.socket_fd, &read_fds)) {
                    processPeerMessage(peer);
                }
            }
        }

        requestPieces();
    }
}

void PeerManager::requestPieces() {
    if (!piece_manager || piece_manager->isComplete()) return;

    for (auto& peer : peers) {
        if (!peer.active || peer.choke) continue;

        // Try to find a piece the peer has and we need
        for (int i = 0; i < piece_manager->getTotalPieces(); ++i) {
            if (peer.pieces_bitfield.size() > (size_t)i && peer.pieces_bitfield[i] && !piece_manager->isPieceDownloaded(i)) {
                
                Block* block = piece_manager->getNextMissingBlock(i);
                if (block) {
                    uint32_t index_net = htonl(i);
                    uint32_t begin_net = htonl(block->offset);
                    uint32_t length_net = htonl(block->length);

                    std::vector<uint8_t> payload(12);
                    std::memcpy(payload.data(), &index_net, 4);
                    std::memcpy(payload.data() + 4, &begin_net, 4);
                    std::memcpy(payload.data() + 8, &length_net, 4);

                    Message request{13, MessageType::REQUEST, payload};
                    Message::writeToSocket(peer.socket_fd, request);
                    block->requested = true;
                    
                    // std::cout << "Requested piece " << i << " offset " << block->offset << " from " << peer.ip << std::endl;
                    return; 
                }
            }
        }
    }
}
