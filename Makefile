all:
	$(CXX) -Ofast -flto -Wall -g2 \
	chrdef.cpp \
	segdata.cpp \
	snpdata.cpp \
	tokenreader.cpp \
	segannot.cpp \
	main.cpp \
	-o gcop -lz

gcop:
	$(CXX) -flto -Wall -g2 \
	chrdef.cpp \
	segdata.cpp \
	snpdata.cpp \
	tokenreader.cpp \
	int/cmdparam.cpp \
	int/command.cpp \
	int/scriptenv.cpp \
	int/objspace.cpp \
	int/objs.cpp \
	int/interpreter.cpp \
	segannot.cpp \
	-o gcop -lz
