#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <curl/curl.h>
#include "token.h"
#include "torrent.h"

#define MAXSIZE 128*1024*1024

struct mem {
	char* string;
	size_t len;
};

size_t custom_curl_write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
	if (!userp)
		return 0;

	size_t length = size * nmemb; 
	struct mem *dest = (struct mem*) userp;
	if (length > MAXSIZE-(dest->len))
		length = MAXSIZE-(dest->len);

	memcpy(&dest->string[dest->len], buffer, length);
	dest->string[dest->len+length] = 0;
	dest->len += length;
	return length;
}


// WARNING! side effect here !
char *get_until_close(char *pos) {
        size_t len = 0;
        while (*pos++ != '>') {
        	len++;
        }
        char* name = malloc(len);
        memcpy(name, pos-len, len);
        name[len-1] = 0;

        return name;
}


struct torrent_list* parse_torrents(char* data) {
	char *begin = strstr(data, "<item>");
	struct torrent_list* list = malloc(sizeof(struct torrent_list));
	list->next = NULL;
	list->tor = NULL;
	while (begin) {
		struct token *tok = get_token(begin);
        struct torrent* tor = torrent_from_token(tok);
        if (tor) {
            printf("%s - %zi - %li \n", tor->title, tor->size, tor->age);
        }
		add_torrent_to_list(list, tor);
        free_token(tok);
		begin = strstr(begin+1, "<item>");
	}

	return NULL;
}

struct torrent_list* filter_torrents(char* url) {
	// 128kB of data (hope this is enought, but anyway curl should pass at most 16kB of data to the WRITEFUNCTION callback at a time)
	struct mem* data = malloc(sizeof(struct mem));
	char *ptr = malloc(MAXSIZE);
	if (!data || !ptr) {
		perror("Couldn't allocate memory! exiting !");
		abort();
	}
	data->string = ptr;
	memset(data->string, 0, MAXSIZE); 
	data->len = 0;

	CURL *curl = curl_easy_init();
	if(curl && url) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, custom_curl_write_data); 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		CURLcode res;
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			perror("Something bad happened with curl !");

		curl_easy_cleanup(curl);
	} else {
		free(data);
		return NULL;
	}
	struct torrent_list* list = parse_torrents(data->string);	
	free(data->string);
	free(data);
	return list;
}

int main(int argc, char* argv[]) {
	//for (;;) {
		filter_torrents("http://www.t411.ch/rss/?cat=408");	
	//	sleep(300);
	//}
	return EXIT_SUCCESS;
}
