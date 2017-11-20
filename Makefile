all:
	$(CXX) -Ofast -flto -Wall -g2 \
	chrdef.cpp \
	segdata.cpp \
	snpdata.cpp \
	tokenreader.cpp \
	segannot.cpp \
	main.cpp \
	-o gcop -lz

int:
	$(CXX) -flto -Wall -g2 \
	chrdef.cpp \
	segdata.cpp \
	snpdata.cpp \
	tokenreader.cpp \
	interpreter.cpp \
	segannot.cpp \
	-o int -lz
