TOP = .
CC = gcc
OPT =
INC = $(TOP)/libcyaml/include
LIB = -lSDL2 -lSDL2_ttf -lm -lyaml -ldeflate
CFLAGS = $(OPT) -D__STDC_FORMAT_MACROS -Wall

OBJS = $(TOP)/obj/dptview.o \
	$(TOP)/obj/options.o \
	$(TOP)/obj/trace_handler.o \
	$(TOP)/obj/trace_gem.o \
	$(TOP)/obj/gfx.o \
	$(TOP)/obj/event.o \
	$(TOP)/obj/array.o \
	$(TOP)/obj/search.o \
	$(TOP)/obj/yaml.o

all: OPT = -O3
all: YAML_PATH = $(TOP)/libcyaml/build/release/static/src
all: $(TOP)/bin/dptview
all: $(shell DPTVFONT=$(TOP)/assets)
all: $(shell export DPTVFONT)

debug: OPT = -O0 -g -fno-inline -fno-unroll-loops
debug: YAML_PATH = $(TOP)/libcyaml/build/debug/static/src
debug: $(TOP)/bin/dptview
debug: $(shell DPTVFONT=$(TOP)/assets)
debug: $(shell export DPTVFONT)

debug: YAML_OBJS = $(YAML_PATH)/copy.o \
			$(YAML_PATH)/free.o \
			$(YAML_PATH)/load.o \
			$(YAML_PATH)/mem.o \
			$(YAML_PATH)/save.o \
			$(YAML_PATH)/utf8.o \
			$(YAML_PATH)/util.o

all: YAML_OBJS = $(YAML_PATH)/copy.o \
			$(YAML_PATH)/free.o \
			$(YAML_PATH)/load.o \
			$(YAML_PATH)/mem.o \
			$(YAML_PATH)/save.o \
			$(YAML_PATH)/utf8.o \
			$(YAML_PATH)/util.o


$(TOP)/bin/dptview: $(OBJS)
	$(CC) $(CFLAGS) -o $(TOP)/bin/dptview $(YAML_OBJS) $(OBJS) $(LIB) 

$(TOP)/obj/dptview.o : $(TOP)/src/dptview.c $(TOP)/src/dptv.h $(TOP)/src/options.h $(TOP)/src/trace_handler.h $(TOP)/src/gfx.h $(TOP)/src/search.h
	$(CC) $(CFLAGS) -c $(TOP)/src/dptview.c -o $(TOP)/obj/dptview.o -I $(INC)

$(TOP)/obj/options.o : $(TOP)/src/options.c $(TOP)/src/dptv.h $(TOP)/src/options.h $(TOP)/src/gfx.h
	$(CC) $(CFLAGS) -c $(TOP)/src/options.c -o $(TOP)/obj/options.o -I $(INC)

$(TOP)/obj/trace_handler.o : $(TOP)/src/trace_handler.c $(TOP)/src/trace_handler.h $(TOP)/src/options.h $(TOP)/src/dptv.h $(TOP)/src/trace_gem.h
	$(CC) $(CFLAGS) -c $(TOP)/src/trace_handler.c -o $(TOP)/obj/trace_handler.o -I $(INC)

$(TOP)/obj/trace_gem.o : $(TOP)/src/trace_gem.c $(TOP)/src/trace_gem.h $(TOP)/src/options.h $(TOP)/src/dptv.h $(TOP)/src/trace_handler.h
	$(CC) $(CFLAGS) -c $(TOP)/src/trace_gem.c -o $(TOP)/obj/trace_gem.o -I $(INC)

$(TOP)/obj/gfx.o : $(TOP)/src/gfx.c $(TOP)/src/options.h $(TOP)/src/dptv.h $(TOP)/src/gfx.h $(TOP)/src/event.h $(TOP)/src/options.h $(TOP)/src/help_text.h $(TOP)/src/search.h
	$(CC) $(CFLAGS) -c $(TOP)/src/gfx.c -o $(TOP)/obj/gfx.o -I $(INC)

$(TOP)/obj/search.o : $(TOP)/src/search.c $(TOP)/src/search.h $(TOP)/src/dptv.h $(TOP)/src/options.h $(TOP)/src/dptv.h $(TOP)/src/gfx.h $(TOP)/src/event.h
	$(CC) $(CFLAGS) -c $(TOP)/src/search.c -o $(TOP)/obj/search.o -I $(INC)

$(TOP)/obj/event.o : $(TOP)/src/event.c $(TOP)/src/options.h $(TOP)/src/dptv.h $(TOP)/src/gfx.h $(TOP)/src/search.h
	$(CC) $(CFLAGS) -c $(TOP)/src/event.c -o $(TOP)/obj/event.o -I $(INC)

$(TOP)/obj/yaml.o : $(TOP)/src/yaml.c $(TOP)/src/yaml.h
	$(CC) $(CFLAGS) -c $(TOP)/src/yaml.c -o $(TOP)/obj/yaml.o -I $(INC)

$(TOP)/obj/array.o : $(TOP)/src/array.c $(TOP)/src/array.h
	$(CC) $(CFLAGS) -c $(TOP)/src/array.c -o $(TOP)/obj/array.o -I $(INC)

clean:
	rm -f $(TOP)/obj/*.o $(TOP)/bin/dptview
