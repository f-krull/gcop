export LIBZA := $(shell pwd)/src/3rdparty/zlib/zlib-1.2.11/build/libz.a

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

CXXFLAGS+= -Wall -g -Ofast -march=native -isystem $(ZLIB) -static
CPPFLAGS+= -std=c++11
LDFLAGS += -Wall -Ofast -static


all: gcop

disreg: libgcop.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) prj/disreg/disreg.cpp -I./src -L. -lgcop $(LIBZA) -o bin/disreg

libgcop.a: $(OBJECTS)
	$(AR) rcs libgcop.a $(OBJECTS)

gcop: $(OBJECTS) $(LIBZA) src/main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) src/main.o $(LIBZA) -o bin/gcop

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: libgcop.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DDEBUG -g2 -Wall src/data/intervaltree_test.cpp -o src/bin/intervaltree_test
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DDEBUG -g2 -Wall src/test/flatten_test.cpp -L. -lgcop -lz -o src/test/flatten_test

distclean: clean
	$(RM) *~ src/.depend
	$(MAKE) -C src/3rdparty/zlib clean

src/.depend: $(SRCS) | libs
	$(RM) src/.depend
	$(CXX) $(CPPFLAGS) -MM $^>>src/.depend;

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
    -include src/.depend
endif
endif

clean:
	$(RM) bin/*
	$(RM) prj/*/*.o src/*.o src/*/*.o
	$(RM) libgcop.a

libs: $(LIBZA)

$(LIBZA):
	$(MAKE) -C src/3rdparty/zlib

.PHONY: test libs
