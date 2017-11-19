all:
	$(CXX) -Ofast -flto -Wall -g2 \
	chrdef.cpp \
	segdata.cpp \
	snpdata.cpp \
	tokenreader.cpp \
	main.cpp \
	-o gcop -lz

int:
	$(CXX) -flto -Wall -g2 \
	interpreter.cpp \
	-o int -lz
