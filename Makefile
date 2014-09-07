CC=gcc
CFLAGS=-Wall -Werror -pthread
LDFLAGS=
INC=
DIRS=bin/ obj/
DIRSTAMPS := $(addsuffix .dirstamp,$(DIRS))
SOURCES := $(wildcard src/*.c)
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.c=.o)))
EXECUTABLE=bin/battery

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c $(DIRSTAMPS)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(DIRSTAMPS):
	mkdir -p $(@D)
	touch $@

clean:
	rm -rf obj bin