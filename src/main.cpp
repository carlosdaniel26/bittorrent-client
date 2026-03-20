#include "Torrent.h"
#include "Tracker.h"
#include "helpers.h"
#include <openssl/sha.h>

#include <iostream>

int main()
{
    Torrent torrent = Torrent::fromFile("torrents/debian.torrent");
    torrent.printInfo();


    std::string peer_id = "-CT1000-" + generateRandomID(12); // 20 bytes total

    Tracker tracker(torrent.announce);
    std::vector<Peer> peers = tracker.getPeers(torrent.info_hash, peer_id, 6881, 0, 0, torrent.length);

    for (const auto& peer : peers) {
        std::cout << "Peer: " << peer.ip << ":" << peer.port << std::endl;
    }
    return 0;
}