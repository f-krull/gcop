ZLIB   = zlib-1.2.11
LIBZA ?= zlib-1.2.11/build/libz.a

all: $(LIBZA)

download: $(ZLIB).tar.gz

$(ZLIB).tar.gz:
	wget https://github.com/madler/zlib/archive/v1.2.11.tar.gz -O $(ZLIB).tar.gz

$(LIBZA): download
	tar xf $(ZLIB).tar.gz \
	  && mkdir -p $(ZLIB)/build \
	  && cd $(ZLIB)/build && ../configure
	  $(MAKE) -C $(ZLIB)/build

clean:
	
distclean: clean
	$(RM) -r $(ZLIB)

dlclean: 
	$(RM) $(ZLIB).tar.gz

.PHONY: clean distclean download dlclean
