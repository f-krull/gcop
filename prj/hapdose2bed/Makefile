BIN=$(BINDIR)/hapdose2bed

SRCS=main.cpp \
     l_buffer.cpp \
     infofile.cpp \
     bedwriter.cpp \
     famwriter.cpp \
     hapdosereader.cpp
OBJS =$(subst .cpp,.o,$(SRCS))

all: $(BIN)

$(BIN): $(LIBZA) $(OBJS)
	$(CXX) $(LDFLAGS) -o $(BIN) $(OBJS) $(LDLIBS) $(LIBZA)
