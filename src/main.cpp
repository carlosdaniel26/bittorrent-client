#include "Torrent.h"
#include <iostream>

int main()
{
    Torrent torrent = Torrent::fromFile("torrents/debian.torrent");
    torrent.printInfo();
    return 0;
}