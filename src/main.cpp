#include "Torrent.h"
#include "Tracker.h"
#include "helpers.h"
#include "PeerManager.h"
#include "PieceManager.h"
#include "FileManager.h"
#include <openssl/sha.h>
#include <unistd.h>
#include <cstdlib>

#include <iostream>

int main(int argc, char* argv[])
{
    std::srand(std::time(nullptr));
    std::string torrent_file = "torrents/movie.torrent";
    if (argc > 1) {
        torrent_file = argv[1];
    }
    Torrent torrent = Torrent::fromFile(torrent_file);
    torrent.printInfo();


    std::string peer_id = generatePeerID();

    auto working_tracker = torrent.selectWorkingTracker(peer_id);
    if (working_tracker == std::nullopt) {
        std::cerr << "No working tracker found!" << std::endl;
        return 1;
    }
    Tracker tracker = Tracker(working_tracker.value());
    std::vector<Peer> peers;
    try {
        peers = tracker.getPeers(torrent.info_hash, peer_id, 6881, 0, 0, torrent.length);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching peers from tracker: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Tracker returned " << peers.size() << " peers" << std::endl;

    std::cout << "Initializing PieceManager: size=" << torrent.total_size << " piece_len=" << torrent.piece_length << std::endl;
    PieceManager piece_manager(torrent.total_size, (int)torrent.piece_length);
    std::cout << "PieceManager initialized with " << piece_manager.getTotalPieces() << " pieces" << std::endl;

    FileManager file_manager(torrent);
    piece_manager.setFileManager(&file_manager);

    /*try handshake*/
    PeerManager peer_manager(torrent.info_hash, &piece_manager);
    std::cout << "Handshaking with peers (up to 5 successful)..." << std::endl;
    int successful_connections = 0;
    for (auto& peer : peers) {
        if (peer_manager.connectToPeer(peer)) {
            std::cout << "Connected to " << peer.ip << ":" << peer.port << std::endl;
            if (peer_manager.sendHandshake(peer, peer_id) && peer_manager.receiveHandshake(peer)) {
                std::cout << "Handshake successful with " << peer.ip << std::endl;
                peer_manager.addPeer(peer);
                // Send interested
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

    std::cout << "Starting message loop..." << std::endl;
    peer_manager.startConnections();

    return 0;
}
