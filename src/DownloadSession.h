#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "Torrent.h"
#include "Tracker.h"
#include "PieceManager.h"
#include "FileManager.h"
#include "PeerManager.h"

class DownloadSession {
public:
    DownloadSession(const std::string& torrent_file_path);
    ~DownloadSession() = default;

    bool start();
    void run();

private:
    std::string peer_id;
    Torrent torrent;
    std::unique_ptr<Tracker> tracker;
    std::unique_ptr<FileManager> file_manager;
    std::unique_ptr<PieceManager> piece_manager;
    std::unique_ptr<PeerManager> peer_manager;

    bool initTracker();
    bool initManagers();
    bool connectToPeers();
};
