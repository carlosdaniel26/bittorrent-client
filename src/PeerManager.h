#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <sys/socket.h>
#include <cstdbool>

#include "Peer.h"
#include "Piece.h"
#include "Torrent.h"
#include "Messages.h"

#include "PieceManager.h"

class PeerManager {
private:
    std::vector<Peer> peers;
    InfoHash info_hash;

    PieceManager* piece_manager = nullptr;

public:
    PeerManager(const InfoHash& hash, PieceManager* pm) : info_hash(hash), piece_manager(pm) {}
    PeerManager() = default;

    void addPeer(const Peer& peer);
    void addPeers(const std::vector<Peer>& peers);
    
    bool connectToPeer(Peer& peer, int timeout_sec = 5);
    bool sendHandshake(Peer& peer, const std::string& client_peer_id);
    bool receiveHandshake(Peer& peer, int timeout_sec = 15);

    void startConnections();
    void requestPieces();

    void processPeerMessage(Peer& peer);
};