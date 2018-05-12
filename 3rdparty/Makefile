TINYSHA1=TinySHA1.hpp
CIMG=CImg.h


all: $(TINYSHA1) $(CIMG)


$(TINYSHA1):
	wget https://raw.githubusercontent.com/ipkn/crow/master/include/crow/TinySHA1.hpp -O $(TINYSHA1)

$(CIMG):
	wget https://raw.githubusercontent.com/dtschump/CImg/master/CImg.h -O $(CIMG)

clean:

distclean:
	$(RM) $(TINYSHA1) $(CIMG)
