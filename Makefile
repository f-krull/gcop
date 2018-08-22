prjs = src/prj/hapdose2bed \
      src/prj/gcop \
      src/prj/hmview

libs = src/libgcop

all: $(prjs)

$(prjs):
	#$(MAKE) -C $@ LDFLAGS="$(LDFLAGS)" CPPFLAGS="$(CPPFLAGS)" CXXFLAGS="$(CXXFLAGS)"
	$(MAKE) -C $@

distclean: clean
	$(foreach var,$(prjs), $(MAKE) -C $(var) distclean;)
	$(foreach var,$(libs), $(MAKE) -C $(var) distclean;)

clean:
	$(foreach var,$(prjs), $(MAKE) -C $(var) clean;)
	$(foreach var,$(libs), $(MAKE) -C $(var) clean;)

.PHONY: $(prjs) $(libs)
