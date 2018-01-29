OBJS := data/chrdef.cpp \
				data/chrinfo.cpp \
				data/fieldformat.cpp \
				data/fieldtypes.cpp \
				data/gcords.cpp \
				data/ldinfo.cpp \
				data/tabfield.cpp \
				int/cmd_base.cpp \
				int/cmdparam.cpp \
				int/command.cpp \
				int/objs.cpp \
				int/objspace.cpp \
				int/scriptenv.cpp \
				util/file.cpp \
				util/timer.cpp \
				util/tokenreader.cpp \
				main.cpp

gcop:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -std=c++11 -flto -Wall -g2 $(OBJS) -o gcop -lz

clean:
	$(RM) gcop intervaltree_test

test:
	$(CXX) -g2 -Wall data/intervaltree_test.cpp -o intervaltree_test
