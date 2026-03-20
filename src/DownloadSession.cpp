#include "DownloadSession.h"
#include "helpers.h"
#include "Messages.h"

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

DownloadSession::DownloadSession(const std::string& torrent_file_path) {
    std::srand(std::time(nullptr));
    peer_id = generatePeerID();
    torrent = Torrent::fromFile(torrent_file_path);
}

bool DownloadSession::start() {
    torrent.printInfo();

    if (!initTracker()) {
        std::cerr << "Failed to initialize tracker." << std::endl;
        return false;
    }

    if (!initManagers()) {
        std::cerr << "Failed to initialize managers." << std::endl;
        return false;
    }

    if (!connectToPeers()) {
        std::cerr << "Failed to connect to any peers." << std::endl;
        return false;
    }

    return true;
}

void DownloadSession::run() {
    std::cout << "Starting message loop..." << std::endl;
    if (peer_manager) {
        peer_manager->startConnections();
    }
}

bool DownloadSession::initTracker() {
    auto working_tracker = torrent.selectWorkingTracker(peer_id);
    if (!working_tracker) {
        return false;
    }
    tracker = std::make_unique<Tracker>(*working_tracker);
    return true;
}

bool DownloadSession::initManagers() {
    std::cout << "Initializing PieceManager: size=" << torrent.total_size << " piece_len=" << torrent.piece_length << std::endl;
    piece_manager = std::make_unique<PieceManager>(torrent.total_size, (int)torrent.piece_length);
    std::cout << "PieceManager initialized with " << piece_manager->getTotalPieces() << " pieces" << std::endl;

    file_manager = std::make_unique<FileManager>(torrent);
    piece_manager->setFileManager(file_manager.get());

    peer_manager = std::make_unique<PeerManager>(torrent.info_hash, piece_manager.get());
    return true;
}

bool DownloadSession::connectToPeers() {
    std::vector<Peer> peers;
    try {
        peers = tracker->getPeers(torrent.info_hash, peer_id, 6881, 0, 0, torrent.length);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching peers from tracker: " << e.what() << std::endl;
        return false;
    }

    std::cout << "Tracker returned " << peers.size() << " peers" << std::endl;
    std::cout << "Handshaking with peers (up to 5 successful)..." << std::endl;
    
    int successful_connections = 0;
    for (auto& peer : peers) {
        if (peer_manager->connectToPeer(peer)) {
            std::cout << "Connected to " << peer.ip << ":" << peer.port << std::endl;
            if (peer_manager->sendHandshake(peer, peer_id) && peer_manager->receiveHandshake(peer)) {
                std::cout << "Handshake successful with " << peer.ip << std::endl;
                peer_manager->addPeer(peer);
                
                Message interested{1, MessageType::INTERESTED, {}};
                Message::writeToSocket(peer.socket_fd, interested);
                
                successful_connections++;
                if (successful_connections >= 5) break;
            } else {
                std::cout << "Handshake failed with " << peer.ip << std::endl;
                close(peer.socket_fd);
                peer.socket_fd = -1;
                peer.active = false;
            }
        } else {
            std::cout << "Failed to connect to " << peer.ip << ":" << peer.port << std::endl;
        }
    }
    
    return successful_connections > 0;
}
