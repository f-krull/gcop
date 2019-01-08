BINNAME=hmview
SRCS=main.cpp \
	server.cpp \
	httpservice.cpp \
	wsservice.cpp \
	l_buffer.cpp \
	debuglog.cpp \
	wsmatview.cpp \
	hmmat.cpp \
	distmatrix.cpp \
	dendrogram.cpp \
	clusterersl.cpp \
	clusterercl.cpp \
	urlgeneinfo.cpp
SRCS:=$(addprefix src/, $(SRCS))

PHONIES += 3rdparty
BINDEPS += 3rdparty
DISTCLEANDIRS += 3rdparty
DOWNLOADIRS   += 3rdparty
DLCLEANDIRS   += 3rdparty

#-------------------------------------------------------------------------------

include ../../../makefile.mk

#-------------------------------------------------------------------------------

CPPFLAGS += -I$(shell pwd)/3rdparty/libjpeg/include/
LIBJPEG=$(shell pwd)/3rdparty/libjpeg/lib/libjpeg.a
LDLIBS  += -pthread -lrt $(LIBJPEG)

#-------------------------------------------------------------------------------

all: $(TARGETS)

3rdparty:
	$(MAKE) -C 3rdparty

#-------------------------------------------------------------------------------

buffer_test: buffer.o buffer_test.o
	$(CXX) $(LDFLAGS) buffer.o buffer_test.o -o buffer_test

hmmat_test: hmmat.o hmmat_test.o
	$(CXX) $(LDFLAGS) hmmat.o hmmat_test.o -o hmmat_test

correlationt_test: src/correlation_test.o
	$(CXX) $(LDFLAGS) src/correlation_test.o -o correlationt_test
