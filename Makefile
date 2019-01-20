CFLAGS = -Wall -Wextra -Werror

PROGRAMS = z
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all test clean
.SUFFIXES:
.SUFFIXES: .c .o

all: $(PROGRAMS)

test: all
	prove -v ./t

clean:
	$(RM) $(PROGRAMS) $(OBJECTS)

$(PROGRAMS): $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
