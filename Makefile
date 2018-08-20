SRCS := src/data/chrinfo.cpp \
        src/data/fieldformat.cpp \
        src/data/fieldtypes.cpp \
        src/data/gcords.cpp \
        src/data/tabfield.cpp \
        src/int/cmd_base.cpp \
        src/int/cmdparam.cpp \
        src/int/command.cpp \
        src/int/objs.cpp \
        src/int/objs_base.cpp \
        src/int/objspace.cpp \
        src/int/scriptenv.cpp \
        src/util/file.cpp \
        src/util/timer.cpp \
        src/util/tokenreader.cpp

OBJECTS = $(patsubst %.cpp, %.o, $(SRCS))
HEADERS = $(patsubst %.cpp, %.h, $(SRCS))

CXXFLAGS ?= -std=c++11 -Ofast -Wall -g2

all: gcop


disreg: libgcop.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) prj/disreg/disreg.cpp -I./src -L. -lgcop -lz -o bin/disreg

libgcop.a: $(OBJECTS)
	$(AR) rcs libgcop.a $(OBJECTS)

gcop: $(OBJECTS) src/main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) src/main.o -lz -o bin/gcop

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) bin/*
	$(RM) prj/*/*.o src/*.o src/*/*.o
	$(RM) libgcop.a

test: libgcop.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DDEBUG -g2 -Wall src/data/intervaltree_test.cpp -o src/bin/intervaltree_test
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DDEBUG -g2 -Wall src/test/flatten_test.cpp -L. -lgcop -lz -o src/test/flatten_test

.PHONY: test
