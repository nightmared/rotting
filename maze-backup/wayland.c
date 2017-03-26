#include "wayland.h"

/*** LISTENERS FUNCTIONS ***/

static struct globals globals = { NULL, NULL, NULL, NULL, NULL, WL_SHM_FORMAT_XRGB8888 };

/** SHM **/

static void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format) {
    struct globals *glob = data;
    glob->shm_format = format;
}

struct wl_shm_listener shm_listener = { shm_format };

/** XDG_SHELL **/
static void xdg_shell_ping(void *data, struct xdg_shell *shell, uint32_t serial) {
    xdg_shell_pong(shell, serial);
}

static const struct xdg_shell_listener xdg_shell_listener = {
    xdg_shell_ping,
 };

/** XDG_SURFACE **/
static void xdg_surface_configure(void *data, struct xdg_surface *shell_surface, int32_t width,
                                  int32_t height, struct wl_array *states, uint32_t serial) {
    if (width == 0 || height == 0)
        return;

    struct window *win = data;
    // Resize the window ?
    if (win->width != width || win->height != height) {
        window_set_size(win, width, height);
        if (create_buffer(win) != EXIT_SUCCESS)
            return;
        // trigger the 'release' event
        wl_surface_attach(win->surface, win->buf->wl_buffer, 0, 0);
    }
}

static void xdg_surface_close(void *data, struct xdg_surface *shell_surface) {
    struct window *win = data;
    win->running = 0;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    xdg_surface_configure,
    xdg_surface_close
};

/** WL_BUFFER **/
static void buffer_release(void *data, struct wl_buffer *wl_buf) {
    struct buffer *buf = data;
    buffer_destroy(buf);
}

static const struct wl_buffer_listener buffer_listener = { buffer_release };



/** REGISTRY **/
static void registry_add(void *data, struct wl_registry *registry, uint32_t id,
                         const char *interface, uint32_t version) {
    struct globals *glob = data;

    if (strcmp(interface, "wl_compositor") == 0) {
        globals.compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    } else if (strcmp(interface, "xdg_shell") == 0) {
        globals.shell = wl_registry_bind(registry, id, &xdg_shell_interface, 1);
        xdg_shell_use_unstable_version(glob->shell, XDG_VERSION);
        xdg_shell_add_listener(glob->shell, &xdg_shell_listener, NULL);
     } else if (strcmp(interface, "wl_shm") == 0) {
        globals.shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
        wl_shm_add_listener(globals.shm, &shm_listener, &globals);
    }
}

static void registry_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
}

static const struct wl_registry_listener registry_listener = {
    registry_add,
    registry_remove
};

/*** API **/

/** REGISTRY LOADING **/
struct globals* load_globals(void) {
    struct wl_display *display = wl_display_connect(NULL);
    if (display == NULL) {
        fprintf(stderr, "Can't connect to wayland display\n");
        return NULL;
    }
    globals.display = display;

    globals.registry = wl_display_get_registry(display);
    wl_registry_add_listener(globals.registry, &registry_listener, &globals);

    wl_display_dispatch(display);
    wl_display_roundtrip(display);
    return &globals;
}

void unload_globals(struct globals *glob) {
    if (glob->shm)
        wl_shm_destroy(glob->shm);
    if (glob->shell)
        xdg_shell_destroy(glob->shell);
    if (glob->compositor)
        wl_compositor_destroy(glob->compositor);
    if (glob->display)
        wl_display_disconnect(glob->display);
}

/** WINDOWS MANAGEMENT **/
struct window* create_window(struct globals *glob) {
    if (glob == NULL)
        return NULL;

    struct window *win = malloc(sizeof(struct window));
    win->width = 0;
    win->height = 0;
    win->globals = glob;
    win->running = 1;

    win->surface = wl_compositor_create_surface(glob->compositor);
    if (win->surface == NULL) {
        fprintf(stderr, "Can't create surface\n");
        window_destroy(win);
        return NULL;
    }

    win->xdg_surface = xdg_shell_get_xdg_surface(glob->shell, win->surface);
    if (win->xdg_surface == NULL) {
        fprintf(stderr, "Can't create xdg_surface, does the compositor handle xdg shell ?\n");
        window_destroy(win);
        return NULL;
    }

    xdg_surface_add_listener(win->xdg_surface, &xdg_surface_listener, win);

    return win;
}


void window_set_size(struct window *win, int width, int height) {
    if (win == NULL)
        return;

    win->width = width;
    win->height = height;
}


void window_set_title(struct window *win, char *name) {
    if (win == NULL)
        return;

    xdg_surface_set_title(win->xdg_surface, name);
}

void window_destroy(struct window *win) {
    if (win == NULL)
        return;

    if (win->surface)
        wl_surface_destroy(win->surface);
    if (win->xdg_surface)
        xdg_surface_destroy(win->xdg_surface);
    if (win->buf)
        buffer_destroy(win->buf);
}

/** BUFFERS HANDLING **/
int create_buffer(struct window *win) {
    if (win == NULL || win->globals->shm == NULL)
        return EXIT_FAILURE;

    uint32_t stride = win->width*sizeof(uint32_t);
    uint32_t size = win->height*stride+sizeof(uint32_t);

    int fd = shm_open("/maze-0", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1 || ftruncate(fd, size) == -1)
        return EXIT_FAILURE;

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %m\n");
        close(fd);
        shm_unlink("/maze-0");
        return EXIT_FAILURE;
    }

    shm_unlink("/maze-0");


    struct wl_shm_pool *pool = wl_shm_create_pool(win->globals->shm, fd, size*2);
    struct wl_buffer *wl_buffer = wl_shm_pool_create_buffer(pool, 0, win->width,
                               win->height, stride, win->globals->shm_format);
    wl_shm_pool_destroy(pool);

    close(fd);
    
    struct buffer *buffer = malloc(sizeof(struct buffer));
    buffer->wl_buffer = wl_buffer;
    buffer->size = size;
    buffer->shm_data = data;
    wl_buffer_add_listener(wl_buffer, &buffer_listener, buffer);
    win->buf = buffer;
    return EXIT_SUCCESS;
}

void buffer_destroy(struct buffer* buf) {
    if (buf == NULL)
        return;
    
    wl_buffer_destroy(buf->wl_buffer);
    munmap(buf->shm_data, buf->size);
    free(buf);
    buf = NULL;
}
