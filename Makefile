# make       -> compile microcoap app
# make clean -> remove the app file and all object files (.o)
# make all   -> clean and compile
APPNAME  = microcoap
SRC     = main-posix.c   \
          libraries/microcoap/microcoap.c \
          microcoap-example/endpoints.c
# compilation options
CFLAGS  = -Wall -DDEBUG
INC_DIR = libraries/microcoap \
          microcoap-example
# -DIPV6
# linking options
# LDFLAGS = -shared -Wl,-soname,$(APPNAME)

# how to compile individual object files
OBJS    = $(SRC:.c=.o)
.c.o:
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

.PHONY: all clean

# library compilation
$(APPNAME): $(OBJS) $(SRC)
	$(CC) $(OBJS) -o $(APPNAME)
#$(LDFLAGS)
    
# cleaning rule
clean:
	rm -f $(OBJS) $(APPNAME) *~

# additional rule
all: clean $(APPNAME)