#include "includes.h"

struct grid {
    uint32_t *data;
    uint32_t width;
};


struct grid* new_grid(uint32_t size);
void free_grid(struct grid* grid);
void grid_insert_data(struct grid *grid, uint32_t x, uint32_t y, uint32_t data);
uint32_t* grid2buffer(struct grid *grid, uint32_t *addr, uint32_t win_width, uint32_t win_height);
/*
 * Parse the file, the file should be structured as follow:
 * - A header of two byte, the first one defining the maze width (it only handle
 *   squares for the moment), and the second one being the LF byte (0x0A). This
 *   means your maze have dimensions of m*m with 0<m<2^32 (0x0 byte isn't allowed
 *   as it would imply a maze of no width, which is kind of a nonsense).
 *
 *  This looks like:
 *    -------------
 *   | width | 0xA |
 *    -------------
 *   0       32    40 (bits)
 *
 * - The various positions, alongsides theirs values. The position-value tuple
 *   is declared as follow:
 *    -----------------------------
 *   |   x   |   y   | value | 0xA |
 *    -----------------------------
 *   0       32      64      96   104 (bits)
 *
 *   Pretty simple, right ? The three items (x and y coordinate, plus the value)
 *   are each stored using 32 bit. Each tuple is suffixed by a LF byte again.
 *   x, y and the value are stored as unsigned 32 bits integers.
 *   The value is store in RGBA format.
 *
 * Every number is stored in big endian format;
 * That's it, folks !
 * So in the end, we have a file with 1 headers and numerous (that means, going
 * from 0 to your computer limits) position-value tuples.
 * If the same position is specified two or more times, the last one (in the file)
 * should be applied.
 */
struct grid* grid_parse_file(char* filepath);
