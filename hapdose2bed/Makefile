CXXFLAGS+= -Wall -g -Ofast -march=native -isystem $(ZLIB) -static
CPPFLAGS+= -std=c++11
LDFLAGS += -Wall -Ofast -static
LDLIBS  += $(ZLIBA)

EXE=hapdose2bed

SRCS=src/main.cpp \
     src/buffer.cpp \
     src/infofile.cpp \
     src/bedwriter.cpp \
     src/famwriter.cpp \
     src/hapdosereader.cpp
OBJS =$(subst .cpp,.o,$(SRCS))

ZLIB  = zlib-1.2.11
ZLIBA = zlib-1.2.11/build/libz.a


all: $(EXE)

$(EXE): $(ZLIBA) $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXE) $(OBJS) $(LDLIBS)

clean:
	$(RM) $(OBJS) $(EXE) src/.depend

distclean: clean
	$(RM) *~ src/.depend
	$(RM) -r $(ZLIB)

src/.depend: $(SRCS) | $(3RDPARTY)
	$(RM) src/.depend
	$(CXX) $(CPPFLAGS) -MM $^>>src/.depend;

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
    -include src/.depend
endif
endif


$(ZLIBA):
	tar xf $(ZLIB).tar.gz && \
	mkdir $(ZLIB)/build && \
	cd $(ZLIB)/build && ../configure
	$(MAKE) -C $(ZLIB)/build


.PHONY:
