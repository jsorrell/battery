CC=gcc
CFLAGS=-Wall -Werror
LDFLAGS=
INC=
DIRS=bin/ obj/
DIRSTAMPS := $(addsuffix .dirstamp,$(DIRS))
SOURCES := $(wildcard src/*.c)
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.c=.o)))
EXECUTABLE=bin/battery

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	src/make_acadapter.sh

install:
	cp bin/acadapter.sh /etc/pm/power.d/acadapter.sh
	chmod 755 /etc/pm/power.d/acadapter.sh
uninstall:
	rm -f /etc/pm/power.d/acadapter.sh

obj/%.o: src/%.c $(DIRSTAMPS)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(DIRSTAMPS):
	mkdir -p $(@D)
	touch $@

clean:
	rm -rf obj bin