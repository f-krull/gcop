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
	segdata.cpp \
	snpdata.cpp \
	data/tokenreader.cpp \
	data/ldinfo.cpp \
	data/gcords.cpp \
	int/cmdparam.cpp \
	int/command.cpp \
	int/scriptenv.cpp \
	int/objspace.cpp \
	int/objs.cpp \
	int/interpreter.cpp \
	util/timer.cpp \
	util/file.cpp \
	segannot.cpp \
	-o gcop -lz
