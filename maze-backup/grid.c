#include "grid.h"

struct grid* new_grid(uint32_t size) {
    if (size == 0)
        return NULL;

    uint32_t *data = malloc(size*size*sizeof(uint32_t));
    struct grid *grid = malloc(sizeof(struct grid));
    memset(data, 0, size*size*sizeof(uint32_t));
    if (data == NULL || grid == NULL) {
        return NULL;
    }
    grid->data = data;
    grid->width = size;
    return grid;
}

void free_grid(struct grid* grid) {
    if (grid == NULL || grid->data == NULL)
        return;

    free(grid->data);
    free(grid);
}

void grid_insert_data(struct grid *grid, uint32_t x, uint32_t y, uint32_t data) {
    if (grid == NULL || grid->data == NULL || x >= grid->width || y >= grid->width)
        return;

    grid->data[x*grid->width+y] = data;
}

uint32_t* grid2buffer(struct grid *grid, uint32_t *addr, uint32_t win_width, uint32_t win_height) {
    if (addr == NULL || grid == NULL || grid->width == 0 || win_width == 0 || win_height == 0)
        return NULL;


/*    // Modulo should return 0, otherwise, some lines wouldn't be used
    if (win_height % grid->width != 0 || win_width % grid->width)
        return NULL;
*/
    uint32_t line_height = win_height / grid->width;
    uint32_t line_width = win_width / grid->width;
    uint32_t pos = 0;

    for(int i = 0; i < grid->width; i++) { 
        for(int j = 0; j < grid->width; j++) {
            uint32_t value = grid->data[i*grid->width+j];
            // pos now points to the beginning of the future square
            pos = i*win_width*line_height+j*line_width;
            // Do the square
            for(int i = 0; i < line_height; i++) {
                for(int j = 0; j < line_width; j++) {
                    addr[pos+i*win_width+j] = value;
                 }
            }
        }
    }
    return addr;
}

uint32_t get_num(uint8_t *buf, uint32_t i) {
    return buf[i] << 24 | buf[i+1] << 16 | buf[i+2] << 8 | buf[i+3];
}

struct grid* grid_parse_file(char* filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Could not open given file. %m.\n");
        return NULL;
    }

    struct grid *maze = NULL;

    // read header
    uint8_t *buf = malloc(5);
    int count = read(fd, buf, 5);
    if (count <= -1) {
        goto exit;
    }
    if (buf[4] != 0xA) {
        fprintf(stderr, "invalid file format !\n");
        goto exit;
    }

    maze = malloc(sizeof(struct grid));
    maze->width = get_num(buf, 0);
    maze->data = malloc(maze->width*maze->width*sizeof(uint32_t));
    if (!maze->data) {
        free(maze);
        maze = NULL;
        goto exit;
    }
    memset(maze->data, 0, maze->width*maze->width*sizeof(uint32_t));

    free(buf);
    buf = malloc(13);
    uint32_t x = 0, y = 0, val = 0;
    while (read(fd, buf, 13) == 13) {
        x = get_num(buf, 0); 
        y = get_num(buf, 4);
        val = get_num(buf, 8);
        maze->data[x*maze->width+y] = val;
    }

exit:
    free(buf);
    close(fd);

    return maze;
}
