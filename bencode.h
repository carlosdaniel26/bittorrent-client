#ifndef BENCODE_H
#define BENCODE_H

typedef struct {
    char* announce;
    char* name;
    long long length;
    long long piece_length;
} TorrentInfo;

#endif