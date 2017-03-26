#ifndef TORRENT_H
#define TORRENT_H

#include <time.h>
#include "token.h"

struct torrent {
    char* title;
    char* url;
    size_t size;
    time_t age;
};

struct torrent_list {
    struct torrent* tor;
    struct torrent_list* next;
};

struct torrent* torrent_from_token(struct token* tok);
void free_torrent(struct torrent* tor);
void add_torrent_to_list(struct torrent_list* list, struct torrent* tor);

#endif
