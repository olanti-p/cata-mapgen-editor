# Compiling

The editor is based on a fork of Dark Days Ahead, and its source code
is integrated into the build. You shouldn't need any additional
configuration, just follow the original compiling guides for DDA.

Only these build system are supported:
- Visual Studio solution in "msvc_full_features/"
- CMake (should work, but not tested)

The game most likely won't build with anything else (e.g. Makefile).

For example, on linux, cmake build can be produced with the following commands:
```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DTESTS=OFF -DTILES=ON -DUSE_HOME_DIR=OFF -DLOCALIZE=OFF -DUSE_PREFIX_DATA_DIR=OFF
make -j16
```

And executed like this (from root directory):
```
build/src/cataclysm-tiles
```
