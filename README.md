# hmview
Server based interactive heatmap visualization

## Get source code
```
git clone https://github.com/precimed/hmview
cd hmview
```

## Download dependencies:
```
make 3rdparty
```

## Compile:
```
make
```

## TODO: 
* drag
* selection info (via tooltip?)
* selection highlight
* normalization
* minimap navigation
* ...
* (fix ServerTcp dtor / ownership of service obj)
* (httpfileservice client request for GET)
* (httpfileservice send Content-Length)
