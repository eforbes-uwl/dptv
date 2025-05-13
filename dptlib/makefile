TOP = .
CC = gcc
OPT =
INC = $(TOP)/src
LIB = 
CFLAGS = $(OPT) -D__STDC_FORMAT_MACROS -Wall

OBJS = $(TOP)/obj/dptv_trace.o 

all: OPT = -O3
all: $(TOP)/lib/libdpt.so

$(TOP)/lib/libdpt.so: $(OBJS)
	$(CC) $(CFLAGS) -shared -o $(TOP)/lib/libdpt.so $(OBJS) $(LIB) 

$(TOP)/obj/dptv_trace.o : $(TOP)/src/dptv_trace.c $(TOP)/src/dptv_trace.h 
	$(CC) $(CFLAGS) -c -fPIC $(TOP)/src/dptv_trace.c -o $(TOP)/obj/dptv_trace.o -I $(INC)

clean:
	rm -f $(TOP)/obj/*.o $(TOP)/lib/libdpt.so
