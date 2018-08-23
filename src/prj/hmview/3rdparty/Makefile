TINYSHA1=TinySHA1.hpp
CIMG=CImg.h
LIBJPEGSRC=jpegsrc.v9c.tar.gz
LIBJPEGDIR=$(CURDIR)/libjpeg/
LIBJPEG=$(LIBJPEGDIR)/lib/libjpeg.a

all: $(TINYSHA1) $(CIMG) $(LIBJPEG)


$(TINYSHA1):
	wget https://raw.githubusercontent.com/ipkn/crow/master/include/crow/TinySHA1.hpp -O $(TINYSHA1)

$(CIMG):
	wget https://raw.githubusercontent.com/dtschump/CImg/master/CImg.h -O $(CIMG)

$(LIBJPEGSRC):
	wget http://www.ijg.org/files/$(LIBJPEGSRC) -O $(LIBJPEGSRC)

$(LIBJPEG): $(LIBJPEGSRC)
	tar xf $(LIBJPEGSRC) \
	  && cd jpeg-9c \
		&& ./configure --prefix=$(LIBJPEGDIR) --disable-shared && $(MAKE) && $(MAKE) install

clean:
	$(RM) -r $(LIBJPEGDIR)

distclean: clean
	$(RM) $(TINYSHA1) $(CIMG) 
	$(RM) $(LIBJPEGSRC)
	$(RM) -r jpeg-9c
