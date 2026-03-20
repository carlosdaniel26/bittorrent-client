#include "Torrent.h"
#include "Tracker.h"
#include "helpers.h"
#include "PeerManager.h"
#include <openssl/sha.h>

#include <iostream>

int main()
{
    Torrent torrent = Torrent::fromFile("torrents/debian.torrent");
    torrent.printInfo();


    std::string peer_id = generatePeerID();

    Tracker tracker(torrent.announce);
    std::vector<Peer> peers = tracker.getPeers(torrent.info_hash, peer_id, 6881, 0, 0, torrent.length);

    // for (const auto& peer : peers) {
    //     std::cout << "Peer: " << peer.ip << ":" << peer.port << std::endl;
    // }

    /*try handshake*/
    PeerManager peer_manager;
    for (auto& peer : peers) {
        if (peer_manager.connectToPeer(peer)) {
            std::cout << "Connected to " << peer.ip << ":" << peer.port << std::endl;
            if (peer_manager.sendHandshake(peer, peer_id)) {
                std::cout << "Handshake sent to " << peer.ip << std::endl;
            } else {
                std::cout << "Failed to send handshake to " << peer.ip << std::endl;
            }

            if (peer_manager.receiveHandshake(peer)) {
                std::cout << "Handshake received from " << peer.ip << std::endl;
                throw std::runtime_error("Handshake successful, exiting for testing");
            } else {
                std::cout << "Failed to receive handshake from " << peer.ip << std::endl;
            }
        } else {
            std::cout << "Failed to connect to " << peer.ip << ":" << peer.port << std::endl;
        }
    }
    return 0;
}