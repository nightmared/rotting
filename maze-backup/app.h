#include "wayland.h"
#include "grid.h"

struct app {
    struct grid *grid;
    struct window *win;
    uint32_t iter;
};

struct app* create_app(uint32_t grid_size);
struct app* create_app_from_grid(struct grid* grid);
void destroy_app(struct app *App);
int run_app(struct app *App);
void redraw(void *data, struct wl_callback *wl_callback, uint32_t callback_data);
static const struct wl_callback_listener cb_listener = {
        redraw
};
