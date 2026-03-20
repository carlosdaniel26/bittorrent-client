#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <sys/socket.h>
#include <cstdbool>

#include "Peer.h"
#include "Piece.h"
#include "Torrent.h"

class PeerManager {
private:
    std::vector<Peer> peers;
    InfoHash info_hash;

    int piece_length;
    int total_pieces;
    std::vector<Piece> pieces;

public:
    void addPeer(const Peer& peer);
    void addPeers(const std::vector<Peer>& peers);
    
    bool connectToPeer(Peer& peer, int timeout_sec = 5);
    bool sendHandshake(Peer& peer, const std::string& client_peer_id);
    bool receiveHandshake(Peer& peer, int timeout_sec = 15);

    void startConnections();
    void requestPieces();
};