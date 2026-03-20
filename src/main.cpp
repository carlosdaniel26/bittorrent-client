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

#include "DownloadSession.h"

int main(int argc, char* argv[])
{
    std::srand(std::time(nullptr));
    std::string torrent_file = "torrents/debian.torrent";
    DownloadSession session(torrent_file);
    session.start();
    session.run();

    return 0;
}
