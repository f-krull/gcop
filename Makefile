export LIBZA   := $(shell pwd)/src/3rdparty/zlib/zlib-1.2.11/build/libz.a
export BINDIR  := $(shell pwd)/bin
export LIBGCOP := $(shell pwd)/libgcop.a

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

CXXFLAGS+= -Wall -g -Ofast -march=native -isystem $(ZLIB) -static -std=c++11
CPPFLAGS+=
LDFLAGS += -Wall -Ofast -static


all: gcop

disreg: $(LIBGCOP)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) prj/disreg/disreg.cpp -I./src $(LIBGCOP) $(LIBZA) -o  $(BINDIR)/disreg

$(LIBGCOP): $(OBJECTS)
	$(AR) rcs $(LIBGCOP) $(OBJECTS)

gcop: $(OBJECTS) $(LIBZA) src/main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) src/main.o $(LIBZA) -o  $(BINDIR)/gcop

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(LIBGCOP)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DDEBUG -g2 -Wall src/data/intervaltree_test.cpp -o $(BINDIR)/intervaltree_test
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DDEBUG -g2 -Wall src/test/flatten_test.cpp $(LIBGCOP) -lz -o $(BINDIR)/flatten_test

prj:
	$(MAKE) -C prj/hapdose2bed LDFLAGS="$(LDFLAGS)" CPPFLAGS="$(CPPFLAGS)" CXXFLAGS="$(CXXFLAGS)"
	$(MAKE) -C prj/hmview      LDFLAGS="$(LDFLAGS)" CPPFLAGS="$(CPPFLAGS)" CXXFLAGS="$(CXXFLAGS)"

libs: $(LIBZA)

$(LIBZA):
	$(MAKE) -C src/3rdparty/zlib

distclean: clean
	$(RM) *~ src/.depend
	$(MAKE) -C src/3rdparty/zlib clean
	$(MAKE) -C prj/hapdose2bed distclean
	$(MAKE) -C prj/hmview      distclean

clean:
	$(RM) $(BINDIR)/*
	$(RM) src/*.o src/*/*.o
	$(RM) $(LIBGCOP)
	$(MAKE) -C prj/hapdose2bed clean
	$(MAKE) -C prj/hmview     clean

src/.depend: $(SRCS) | libs
	$(RM) src/.depend
	$(CXX) $(CPPFLAGS) -MM $^>>src/.depend;

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
    -include src/.depend
endif
endif

.PHONY: test libs prj
