#include "PieceManager.h"

void PieceManager::markPieceDownloaded(int index, const std::vector<unsigned char>& data) {
    if (index < 0 || index >= total_pieces) return;

    pieces[index].data = data;
    pieces[index].downloaded = true;
}

bool PieceManager::isComplete() const {
    for (const auto& piece : pieces) {
        if (!piece.downloaded) return false;
    }
    return true;
}

int PieceManager::piecesRemaining() const {
    int count = 0;
    for (const auto& piece : pieces) {
        if (!piece.downloaded) count++;
    }
    return count;
}