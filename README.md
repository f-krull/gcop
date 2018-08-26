# gcop

Genomic coordinate processing

Various tools for working with biological data. See [src/prj](src/prj).

## Build

`git clone https://github.com/precimed/gcop && cd gcop && make`

## Build dependencies

* g++ cxx11
* make
* wget
* (md5sum)

## Make targets

* `all` - build everything (binaries and libs)
* `clean` - delete binaries and libs
* `distclean` - delete 3rdparty builds
* `download` - download 3rdparty sources
* `dlclean` - delete 3rdparty sources
* (`test`) - TODO
