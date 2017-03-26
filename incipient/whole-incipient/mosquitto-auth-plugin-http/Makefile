NAME      = mosquitto_auth_plugin_http

INC       = -I. -I/usr/include -I/usr/include/json-c/
CFLAGS    = -Wall -Werror -fPIC -std=c11

LIBS      = -lcurl -ljson-c

all: $(NAME).so

$(NAME).so: $(NAME).o
	$(CC) $(CFLAGS) $(INC) -shared $^ -o $@ $(LIBS)

%.o : %.c
	$(CC) -c $(CFLAGS) $(DEBUG) $(INC) $< -o $@

clean:
	rm -f *.o *.so
