#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "Peer.h"
#include "Piece.h"

class FileManager;

class PieceManager {
private:
    std::vector<Piece> pieces;
    int piece_length;
    int total_pieces;
    int64_t total_size;

    const uint32_t BLOCK_SIZE = 16384; // 16 KB

    FileManager* file_manager = nullptr;

public:
    PieceManager(int64_t total_size, int piece_length)
        : piece_length(piece_length),
          total_size(total_size)
    {
        total_pieces = (total_size + piece_length - 1) / piece_length;
        pieces.reserve(total_pieces);

        for (int i = 0; i < total_pieces; ++i) {
            int len = piece_length;
            if (i == total_pieces - 1) {
                int leftover = total_size - (int64_t)i * piece_length;
                if (leftover > 0) len = (int)leftover;
            }

            Piece p{i, len, {}, false, {}};
            
            // Initialize blocks for this piece
            int num_blocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
            for (int j = 0; j < num_blocks; ++j) {
                uint32_t offset = j * BLOCK_SIZE;
                uint32_t b_len = BLOCK_SIZE;
                if (j == num_blocks - 1) {
                    b_len = len - offset;
                }
                p.blocks.push_back(Block{offset, b_len, {}, false, false});
            }
            pieces.push_back(std::move(p));
        }
    }

    void saveBlock(int index, uint32_t offset, const std::vector<uint8_t>& data);
    bool isComplete() const;
    int piecesRemaining() const;
    int getTotalPieces() const { return (int)pieces.size(); }
    void setFileManager(FileManager* fm) { file_manager = fm; }
    
    void printProgressBar() const;
    float getProgressPercentage() const;
    
    bool isPieceDownloaded(int index) const {
        if (index < 0 || index >= (int)pieces.size()) return true;
        return pieces[index].downloaded;
    }
    int getPieceLength(int index) const {
        if (index < 0 || index >= (int)pieces.size()) return 0;
        return pieces[index].length;
    }

    // New helpers for requesting blocks
    Block* getNextMissingBlock(int piece_index);
    void markPieceDownloaded(int index, const std::vector<uint8_t>& data);
};
