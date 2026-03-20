#pragma once

#include <vector>
#include <cstdint>

struct Block {
    uint32_t offset;
    uint32_t length;
    std::vector<uint8_t> data;
    bool downloaded = false;
    bool requested = false;
};

struct Piece {
    int index;
    int length;
    std::vector<uint8_t> data;
    bool downloaded = false;
    std::vector<Block> blocks;
};
