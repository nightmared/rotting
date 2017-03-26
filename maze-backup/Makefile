NAME      = maze
INC       = -I. -I/usr/include
CFLAGS    = -Werror -Wall -std=gnu11 -g
LIBS      = -lwayland-client -lrt

all: $(NAME) utils

$(NAME): main.o xdg-shell-unstable-v5-protocol.o app.o grid.o wayland.o
		$(CC) $(CFLAGS) $(INC) $^ -o $@ $(LIBS)

utils:
	$(CC) $(CFLAGS) $(INC) generator.c -o maze-generator

run: all
	./maze-generator 20 85 > /tmp/rand && ./maze /tmp/rand

%.o : %.c
		$(CC) -c $(CFLAGS) $(INC) $< -o $@

clean:
	rm -f *.o *.so $(NAME)
