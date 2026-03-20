#pragma once

#include <vector>

struct Piece {
    int index;
    int length;
    std::vector<unsigned char> data;
    bool downloaded = false;
};