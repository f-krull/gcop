ZLIB  = zlib-1.2.11
#ZLIBA = zlib-1.2.11/build/libz.a

all: $(LIBZA)
	echo $(LIBZA)

$(LIBZA):
	tar xf $(ZLIB).tar.gz && \
	mkdir $(ZLIB)/build && \
	cd $(ZLIB)/build && ../configure
	$(MAKE) -C $(ZLIB)/build

distclean: clean

clean: clean
	$(RM) -r $(ZLIB)
