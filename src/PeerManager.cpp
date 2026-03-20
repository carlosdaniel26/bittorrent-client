#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <stdexcept>

#include "PeerManager.h"
#include "helpers.h"

void PeerManager::addPeer(const Peer& peer)
{
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
    inet_pton(AF_INET, peer.ip.c_str(), &addr.sin_addr);

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
    ssize_t sent = send(peer.socket_fd, handshake.data(), handshake.size(), 0);
    return sent == (ssize_t)handshake.size();
}

bool PeerManager::receiveHandshake(Peer& peer, int timeout_sec)
{
    char buffer[68];
    size_t total = 0;

    struct timeval tv{timeout_sec, 0};
    setsockopt(peer.socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    while (total < sizeof(buffer)) {
        ssize_t n = recv(peer.socket_fd, buffer + total, sizeof(buffer) - total, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return false;
        }
        if (n == 0) {
            return false; 
        }
        total += n;
    }

    unsigned char pstrlen = static_cast<unsigned char>(buffer[0]);
    if (pstrlen != 19) return false;

    std::string pstr(buffer + 1, buffer + 1 + pstrlen);
    if (pstr != "BitTorrent protocol") return false;

    // reserved bytes (buffer[20..27]) → ignoradas

    std::string received_info_hash(buffer + 28, buffer + 48);
    if (received_info_hash != std::string(info_hash.begin(), info_hash.end())) {
        return false; 
    }

    //peer.peer_id.assign(buffer + 48, buffer + 68);

    return true;
}