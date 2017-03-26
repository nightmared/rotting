#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "torrent.h"

struct enclosure {
    char* url;
    size_t size;
};

// horrible, need to rewrite it when I have the time
struct enclosure* get_enclosure_data(char* prop) {
    struct enclosure* enc = malloc(sizeof(struct enclosure));
    enc->url = NULL;
    size_t count = 0;
    char* pos = prop;
    while (*(pos+count) != '=')
        count++;
    if (!strncmp(pos, "url", 3)) {
        pos += count + 2;
        count = 0;
        while (*(pos+count) != '"')
            count++;
        enc->url = malloc(sizeof(char)*count);
        memcpy(enc->url, pos, count-1);
        enc->url[count-1] = '\0';
        pos += count + 2;
        count = 0;
    } 
    while (*(pos+count) != '=')
        count++;
    if (!strncmp(pos, "length", 6)) {
        pos += count + 2;
        count = 0;
        while (*(pos+count) != '"')
            count++;
        enc->size = atol(pos);            
    }
    return enc;
}

struct torrent* torrent_from_token(struct token* tok) {
    if (!tok)
        return NULL;

    struct torrent* tor = malloc(sizeof(struct torrent));
    struct token_list* next = tok->tokens;
    while (next) {
        if (!cmp_token_to_name(next->tok, "title")) {
            tor->title = malloc(sizeof(char)*(strlen(next->tok->value)+1));
            memcpy(tor->title, next->tok->value, strlen(next->tok->value)+1);
        } else if (!cmp_token_to_name(next->tok, "pubDate")) {
            struct tm tm;
            memset(&tm, 0, sizeof(struct tm));
            // date is encoded like 'Sat, 29 Oct 2016 13:35:53 +0200'
            strptime(next->tok->value, "%a, %d %b %Y %H:%M:%S", &tm);
            tor->age = time(NULL) - mktime(&tm);
        } else if (!cmp_token_to_name(next->tok, "enclosure")) {
            struct enclosure* enc = get_enclosure_data(next->tok->props);
            tor->size = enc->size;
            tor->url = enc->url;
            free (enc);
            // TODO: parse props and add handling of the size !
        }
 
        next = next->next;
    }
    return tor; 
}

void free_torrent(struct torrent* tor) {
    if (!tor)
        return;

    free(tor->title);
    free(tor->url);
    free(tor);
}

void add_torrent_to_list(struct torrent_list* list, struct torrent* tor) {
    if (!list)
        return;

    struct torrent_list* next = list;
    while (next->next != NULL)
        next = next->next;
    
    if (!next->tor) {
       next->tor = tor;
    } else {
        next->next = malloc(sizeof(struct torrent_list));
        next->next->tor = tor;
        next->next->next = NULL;
    }

}
