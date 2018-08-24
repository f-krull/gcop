BASEDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BINDIR  := $(BASEDIR)/bin/
LIBDIR  := $(BASEDIR)/lib/
BINDEPS ?= 
TARGETS ?= 
PHONIES ?=
CLEANDIRS     ?=
DISTCLEANDIRS ?=
DLCLEANDIRS   ?=
DOWNLOADDIRS  ?=  
export LIBZA   = $(BASEDIR)/src/3rdparty/zlib/zlib-1.2.11/build/libz.a
export LIBGCOP = $(BASEDIR)/lib/libgcop.a
SRCS ?= $(wildcard *.cpp)
OBJS  = $(patsubst %.cpp, %.o, $(SRCS))

#-------------------------------------------------------------------------------

CXXFLAGS+= -Wall -g -Ofast -flto -static -march=native -std=c++11
LDFLAGS += -Wall -g -Ofast -flto -static

#-------------------------------------------------------------------------------

ifeq ($(HAS_LIBGCOP),1)
LDLIBS   += $(LIBGCOP)
BINDEPS  += $(LIBGCOP)
CXXFLAGS += -I$(BASEDIR)/src/libgcop/
endif

ifeq ($(HAS_LIBZ),1)
LDLIBS  += $(LIBZA)
BINDEPS += $(LIBZA)
endif

#-------------------------------------------------------------------------------

_all: all # entry target

#-------------------------------------------------------------------------------

# build binary?
ifneq ($(BINNAME),)
BINPATH = $(BINDIR)/$(BINNAME)
TARGETS += $(BINPATH)
$(BINPATH): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(BINPATH) $(OBJS) $(LDLIBS)
ifneq ($(MAKECMDGOALS),download)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),dlclean)
  -include .depend
endif
endif
endif
endif
.depend: $(SRCS) | $(BINDEPS)
	$(RM) .depend
	$(CXX) $(CPPFLAGS) -MM $^>>.depend;
endif

#-------------------------------------------------------------------------------

# build library?
ifneq ($(LIBNAME),)
LIBPATH = $(LIBDIR)/lib$(LIBNAME).a
TARGETS += $(LIBPATH)
$(LIBPATH): $(OBJS)
	$(AR) rcs $(LIBPATH) $(OBJS)
ifneq ($(MAKECMDGOALS),download)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),dlclean)
  -include .depend
endif
endif
endif
endif
.depend: $(SRCS) | $(BINDEPS)
	$(RM) .depend
	$(CXX) $(CPPFLAGS) -MM $^>>.depend;
endif

#-------------------------------------------------------------------------------

download:
	$(foreach var,$(DOWNLOADIRS), $(MAKE) -C $(var) download;)

clean:
	$(RM) $(OBJS) $(BINPATH) $(LIBPATH) .depend
	$(foreach var,$(CLEANDIRS), $(MAKE) -C $(var) clean;)

distclean: clean
	$(foreach var,$(DISTCLEANDIRS), $(MAKE) -C $(var) distclean;)

dlclean:
	$(foreach var,$(DLCLEANDIRS), $(MAKE) -C $(var) dlclean;)

.PHONY: $(PHONIES) download clean distclean dlclean

#-------------------------------------------------------------------------------

$(LIBZA):
	$(MAKE) -C $(BASEDIR)/src/3rdparty/zlib

$(LIBGCOP):
	$(MAKE) -C $(BASEDIR)/src/libgcop/
