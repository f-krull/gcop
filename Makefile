SRCS := data/chrinfo.cpp \
				data/fieldformat.cpp \
				data/fieldtypes.cpp \
				data/gcords.cpp \
				data/ldinfo.cpp \
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
				util/tokenreader.cpp \
				main.cpp

OBJECTS = $(patsubst %.cpp, %.o, $(SRCS))
HEADERS = $(patsubst %.cpp, %.h, $(SRCS))

CXXFLAGS ?= -std=c++11 -Ofast -Wall -g2

all: gcop

gcop: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -lz -o gcop

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) gcop intervaltree_test
	$(RM) */*.o *.o
	
test:
	$(CXX) -DDEBUG -g2 -Wall data/intervaltree_test.cpp -o intervaltree_test
