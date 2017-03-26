#include "app.h"

struct app* base_creation() {
    struct globals *globals = load_globals();

    struct app *App = malloc(sizeof(struct app));
    memset(App, 0, sizeof(struct app));

    App->win = create_window(globals);
    return App;
}

struct app* create_app(uint32_t grid_size) {
    struct app *App = base_creation();
    App->grid = new_grid(grid_size);
    if (App->win == NULL || App->grid == NULL)
        return NULL;

    return App;
}

// From now on, the app handle the grid and its deallocation. So don't try to
// free the grid pat this point or this wil end up in 'double free' !
struct app* create_app_from_grid(struct grid* grid) {
    struct app *App = base_creation();
    App->grid = grid;
    if (App->win == NULL || App->grid == NULL)
        return NULL;

    return App;
}

void destroy_app(struct app *App) {
    free_grid(App->grid);
    window_destroy(App->win);
    unload_globals(App->win->globals);
    free(App->win);
    App->win->running = 0;
}

int run_app(struct app *App) {
    redraw(App, NULL, NULL);
    while (wl_display_dispatch(App->win->globals->display) != -1 && App->win->running) {}
    destroy_app(App);
    return EXIT_FAILURE;
}

void redraw(void *data, struct wl_callback *callback, uint32_t callback_data) {
    struct app *App = data;
    grid2buffer(App->grid, App->win->buf->shm_data, App->win->width, App->win->height);
    wl_surface_attach(App->win->surface, App->win->buf->wl_buffer, 0, 0);
    struct wl_callback *cb = wl_surface_frame(App->win->surface);
    wl_callback_add_listener(cb, &cb_listener, App);
    wl_surface_commit(App->win->surface);
}
