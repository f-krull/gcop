CXXFLAGS+= -Wall -g -Ofast
CPPFLAGS+= -std=c++11
LDFLAGS += -Wall -Ofast
LDLIBS  += -pthread -lrt -ljpeg

EXE=matvis

SRCS=main.cpp \
	server.cpp \
	httpfileservice.cpp \
	wsservice.cpp \
	buffer.cpp \
	debuglog.cpp \
	wsmatview.cpp \
	hmmat.cpp

3RDPARTY=3rdparty
SRCS:=$(addprefix src/, $(SRCS))
OBJS =$(subst .cpp,.o,$(SRCS))


all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXE) $(OBJS) $(LDLIBS)
 
clean:
	$(RM) $(OBJS) $(EXE) src/.depend
	$(MAKE) -C $(3RDPARTY) clean

distclean: clean
	$(RM) *~ src/.depend
	$(MAKE) -C $(3RDPARTY) distclean

depend: src/.depend

src/.depend: $(SRCS)
	$(RM) src/.depend
	$(CXX) $(CPPFLAGS) -MM $^>>src/.depend;

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),$(3RDPARTY))
    -include src/.depend
endif
endif
endif

$(3RDPARTY):
	$(MAKE) -C $(3RDPARTY)

.PHONY: $(3RDPARTY)

buffer_test: buffer.o buffer_test.o
	$(CXX) $(LDFLAGS) buffer.o buffer_test.o -o buffer_test

hmmat_test: hmmat.o hmmat_test.o
	$(CXX) $(LDFLAGS) hmmat.o hmmat_test.o -o hmmat_test 
