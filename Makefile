SRCS := data/chrinfo.cpp \
        data/fieldformat.cpp \
        data/fieldtypes.cpp \
        data/gcords.cpp \
        data/tabfield.cpp \
        int/cmd_base.cpp \
        int/cmdparam.cpp \
        int/command.cpp \
        int/objs.cpp \
        int/objs_base.cpp \
        int/objspace.cpp \
        int/scriptenv.cpp \
        util/file.cpp \
        util/timer.cpp \
        util/tokenreader.cpp

OBJECTS = $(patsubst %.cpp, %.o, $(SRCS))
HEADERS = $(patsubst %.cpp, %.h, $(SRCS))

CXXFLAGS ?= -std=c++11 -Ofast -Wall -g2

all: gcop


disreg: libgcop.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) prj/disreg/disreg.cpp -L. -lgcop -lz -o bin/disreg

libgcop.a: $(OBJECTS)
	$(AR) rcs libgcop.a $(OBJECTS)

gcop: $(OBJECTS) main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) main.o -lz -o bin/gcop

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) bin/*
	$(RM) */*.o *.o
	$(RM) libgcop.a

test: libgcop.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DDEBUG -g2 -Wall data/intervaltree_test.cpp -o bin/intervaltree_test
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DDEBUG -g2 -Wall test/flatten_test.cpp -L. -lgcop -lz -o test/flatten_test

.PHONY: test
