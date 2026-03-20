#include "PieceManager.h"
#include "FileManager.h"
#include <iostream>
#include <cstring>
#include <iomanip>

void PieceManager::saveBlock(int index, uint32_t offset, const std::vector<uint8_t>& data) {
    if (index < 0 || index >= (int)pieces.size()) return;
    
    Piece& piece = pieces[index];
    if (piece.downloaded) return;

    for (auto& block : piece.blocks) {
        if (block.offset == offset) {
            block.data = data;
            block.downloaded = true;
            break;
        }
    }

    // Check if piece is fully downloaded
    bool all_downloaded = true;
    for (const auto& block : piece.blocks) {
        if (!block.downloaded) {
            all_downloaded = false;
            break;
        }
    }

    if (all_downloaded) {
        piece.downloaded = true;
        
        // Assemble piece data from blocks
        std::vector<uint8_t> piece_data;
        piece_data.reserve(piece.length);
        for (const auto& block : piece.blocks) {
            piece_data.insert(piece_data.end(), block.data.begin(), block.data.end());
        }

        if (file_manager) {
            file_manager->writePiece(index, piece_data);
            // Clear block data from memory to save RAM
            for (auto& b : piece.blocks) {
                b.data.clear();
                b.data.shrink_to_fit();
            }
        }
        
        // Piece complete, redraw progress
        printProgressBar();
    }
}

float PieceManager::getProgressPercentage() const {
    int downloaded = 0;
    for (const auto& piece : pieces) {
        if (piece.downloaded) downloaded++;
    }
    return (float)downloaded * 100.0f / (float)pieces.size();
}

void PieceManager::printProgressBar() const {
    int downloaded = 0;
    for (const auto& piece : pieces) {
        if (piece.downloaded) downloaded++;
    }
    
    float percent = (float)downloaded * 100.0f / (float)pieces.size();
    int width = 50;
    int pos = (int)(width * percent / 100.0f);

    std::cout << "\rProgress: [" << std::string(pos, '#') << std::string(width - pos, '-') << "] "
              << std::fixed << std::setprecision(1) << percent << "% "
              << "(" << downloaded << "/" << pieces.size() << " pieces)" << std::flush;
}

Block* PieceManager::getNextMissingBlock(int piece_index) {
    if (piece_index < 0 || piece_index >= (int)pieces.size()) return nullptr;
    
    Piece& piece = pieces[piece_index];
    if (piece.downloaded) return nullptr;

    for (auto& block : piece.blocks) {
        if (!block.downloaded && !block.requested) {
            return &block;
        }
    }
    return nullptr;
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

void PieceManager::markPieceDownloaded(int index, const std::vector<uint8_t>& data) {
    if (index < 0 || index >= (int)pieces.size()) return;
    pieces[index].downloaded = true;
    if (file_manager) {
        file_manager->writePiece(index, data);
    }
    printProgressBar();
}
