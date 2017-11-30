all:
	$(CXX) -Ofast -flto -Wall -g2 \
	data/chrdef.cpp \
	segdata.cpp \
	snpdata.cpp \
	data/tokenreader.cpp \
	segannot.cpp \
	main.cpp \
	-o gcop -lz

gcop:
	$(CXX) -flto -Wall -g2 \
	data/chrdef.cpp \
	data/tokenreader.cpp \
	data/ldinfo.cpp \
	data/gcords.cpp \
	int/cmdparam.cpp \
	int/command.cpp \
	int/cmd_base.cpp \
	int/scriptenv.cpp \
	int/objspace.cpp \
	int/objs.cpp \
	util/timer.cpp \
	util/file.cpp \
	main.cpp \
	-o gcop -lz
