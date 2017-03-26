#include <stdlib.h>
#include <stdio.h>
#include <wayland-client.h>
#include "app.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: '%s filename', where filename is a compliant maze file\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct grid *maze = grid_parse_file(argv[1]);
    struct app *App = create_app_from_grid(maze);
    if (!App)
        return EXIT_FAILURE;

    window_set_size(App->win, 500, 500);
    window_set_title(App->win, "maze");

    if (create_buffer(App->win) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    return run_app(App);
}
