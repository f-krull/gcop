OBJS := data/chrdef.cpp \
				data/gcords.cpp \
				data/ldinfo.cpp \
				data/tokenreader.cpp \
				int/cmd_base.cpp \
				int/cmdparam.cpp \
				int/command.cpp \
				int/objs.cpp \
				int/objspace.cpp \
				int/scriptenv.cpp \
				util/file.cpp \
				util/timer.cpp \
				main.cpp

gcop:
	$(CXX) -flto -Wall -g2 $(OBJS) -o gcop -lz

clean:
	$(RM) gcop
