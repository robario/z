CFLAGS = -Wall -Wextra -Werror
YFLAGS = --defines
LFLAGS = --perf-report

PROGRAMS = z
INTERMEDIATES = $(patsubst %.l,%.c,$(wildcard *.l)) $(addprefix $(basename $(wildcard *.y)),.h .c)
SOURCES = $(filter-out $(INTERMEDIATES),$(wildcard *.l *.y *.c))
OBJECTS = $(filter-out $(SOURCES),$(SOURCES:.l=.o) $(SOURCES:.y=.o) $(SOURCES:.c=.o))
DEPDIR = .deps
DEPENDENCIES = $(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))

.INTERMEDIATE: $(INTERMEDIATES)
.PHONY: all test clean distclean realclean debug
.SUFFIXES:
.SUFFIXES: .l .y .h .c .o

all: $(PROGRAMS)

test: all
	prove -v ./t

clean:
	$(RM) $(PROGRAMS) $(OBJECTS)

distclean: clean
	$(RM) $(DEPENDENCIES) $(INTERMEDIATES)
	$(RM) -d $(DEPDIR)

realclean:
	git ls-files --directory --others -z | xargs -0 rm -frv --

$(PROGRAMS): $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.c %.h: %.y
	$(YACC.y) $<

$(DEPDIR):
	mkdir -p $@

$(DEPDIR)/%.d: %.l $(DEPDIR)
	$(CC) -x c -MM -MT '$(@F) $(@F:.d=.o)' -MG -MP -MF $@ $<

$(DEPDIR)/%.d: %.y $(DEPDIR)
	$(CC) -x c -MM -MT '$(@F) $(@F:.d=.o)' -MG -MP -MF $@ $<

$(DEPDIR)/%.d: %.c $(DEPDIR)
	$(CC) -x c -MM -MT '$(@F) $(@F:.d=.o)' -MG -MP -MF $@ $<

ifneq ($(findstring clean,$(MAKECMDGOALS))$(filter-out clean distclean realclean,$(MAKECMDGOALS)),clean)
-include $(DEPENDENCIES)
endif
