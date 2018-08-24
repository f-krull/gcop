prjs = src/prj/hapdose2bed \
       src/prj/gcop \
       src/prj/hmview \
       src/prj/disreg

libs     = src/libgcop
3rdparty = src/3rdparty/zlib

all: $(prjs)

$(prjs):
	$(MAKE) -C $@

download:
	$(foreach var,$(prjs),     $(MAKE) -C $(var) download;)
	$(foreach var,$(3rdparty), $(MAKE) -C $(var) download;)

clean:
	$(foreach var,$(prjs), $(MAKE) -C $(var) clean;)
	$(foreach var,$(libs), $(MAKE) -C $(var) clean;)

distclean: clean
	$(foreach var,$(prjs),     $(MAKE) -C $(var) distclean;)
	$(foreach var,$(libs),     $(MAKE) -C $(var) distclean;)
	$(foreach var,$(3rdparty), $(MAKE) -C $(var) distclean;)

dlclean: distclean
	$(foreach var,$(prjs),     $(MAKE) -C $(var) dlclean;)
	$(foreach var,$(3rdparty), $(MAKE) -C $(var) dlclean;)

.PHONY: $(prjs) $(libs) $(3rdparty) download clean distclean dlclean
