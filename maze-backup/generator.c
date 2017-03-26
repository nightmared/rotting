#include "includes.h"
#include <time.h>

void write_num(uint8_t *buf, uint32_t num, uint32_t pos) {
    buf[pos] = num >> 24;
    buf[pos+1] = num >> 16;
    buf[pos+2] = num >> 8;
    buf[pos+3] = num;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: '%s size nb_gen > your_maze_file', with size the size of one of the side of the maze, nb_gen the numbers of 'tuples to generate' and your_maze_file the file in wich you cant to store the generated file\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Hope to receive an integer, otherwise: BOOM ! undefined behavior !
    int size = atoi(argv[1]);
    int nb_gen = atoi(argv[2]);

    // Bad crypto, bhooo... (I should probably better read /dev{u}random but...
    // who cares in that case ?)
    srand(time(NULL));

    uint8_t *buffer = malloc(nb_gen*13+5);
    memset(buffer, 0, nb_gen*13+5);

    int pos = 0;

    // Write header
    write_num(buffer, size, pos);
    pos += 4;
    buffer[pos] = 0xA;
    pos++;
    
    // Let's generate nb_gen numbers
    for (int i = 0; i < nb_gen; i++) {
        int x = rand() % size;
        write_num(buffer, x, pos);
        pos += 4;
        int y = rand() % size;
        write_num(buffer, y, pos);
        pos += 4;
        write_num(buffer, 0xffffffff, pos);
        pos += 4;
        buffer[pos] = 0xA;
        pos++;
    }

    for (int i = 0; i < nb_gen*13+5; i++) {
        putchar(buffer[i]);
    }

    return EXIT_SUCCESS;
}
