# hmview
Server based interactive heatmap visualization

## Requirements:
* c++11, make, wget

## Get source code:
```
git clone https://github.com/precimed/hmview
cd hmview
```

## Install dependencies:
```
make 3rdparty
```

## Compile:
```
make
```

## TODO:
* selection highlight
* draw x dendrograms
* normalization
* minimap navigation
* multi selection + tooltip
* ...
* (fix ServerTcp dtor / ownership of service obj)
* (httpfileservice send Content-Length)
