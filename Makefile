all:
	$(CXX) -Ofast -flto -Wall -g2 \
	chrdef.cpp \
	segdata.cpp \
	snpdata.cpp \
	tokenreader.cpp \
	main.cpp \
	-o ptoo -lz