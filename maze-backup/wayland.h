#include <wayland-client.h>
#include <includes.h>

#define XDG_VERSION 5

struct globals {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct xdg_shell *shell;
    struct wl_shm *shm;
    uint32_t shm_format;
};

struct buffer {
    struct wl_buffer *wl_buffer;
    uint32_t *shm_data;
    uint32_t size;
};

struct window {
    struct globals* globals;
    uint16_t width;
    uint16_t height;
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct buffer *buf;
    int running;
};

struct globals* load_globals(void);
void unload_globals(struct globals *glob);
struct window* create_window(struct globals *glob);
void window_set_size(struct window *win, int width, int height);
void window_set_title(struct window *win, char *name);
void window_destroy(struct window *win);
int create_buffer(struct window *win);
void buffer_destroy(struct buffer* buf);
