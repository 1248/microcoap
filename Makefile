CFLAGS += -Wall -DDEBUG
# -DIPV6
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
DEPS = $(SRC:%.c=%.d)
EXEC = coap

all: $(EXEC)

-include $(DEPS)

$(EXEC): $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $^

%.o: %.c %.d
	@$(CC) -c $(CFLAGS) -o $@ $<

%.d: %.c
	@$(CC) -MM $(CFLAGS) $< > $@

clean:
	@$(RM) $(EXEC) $(OBJ) $(DEPS)
