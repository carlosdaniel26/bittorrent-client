#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "Peer.h"
#include "Piece.h"

class PieceManager {
private:

    std::vector<Piece> pieces;
    int piece_length;
    int total_pieces;
    int total_size;

public:
    PieceManager(int total_size, int piece_length)
        : piece_length(piece_length),
          total_size(total_size)
    {
        // Compute total pieces, rounding up
        total_pieces = (total_size + piece_length - 1) / piece_length;
        pieces.reserve(total_pieces);

        for (int i = 0; i < total_pieces; ++i) {
            int len = piece_length;

            // Last piece might be smaller
            if (i == total_pieces - 1) {
                int leftover = total_size - i * piece_length;
                if (leftover > 0) len = leftover;
            }

            pieces.push_back(Piece{i, len});
        }
    }

    void markPieceDownloaded(int index, const std::vector<unsigned char>& data);
    bool isComplete() const;
    int piecesRemaining() const;
};