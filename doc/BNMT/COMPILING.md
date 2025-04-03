# Bright Nights Mapping Tool

## Compiling

The editor is based on a fork of Bright Nights, and its source code
is integrated into the CMake build. You shouldn't need any additional
configuration, just follow the original CMake compiling guide for BN.

These build systems are NOT supported and WON'T work:
- Android build
- Makefile
- VS project in "msvc_full_features/"

If you want to use one of them (for some reason..?), you'll have to add
rules for compiling editor sources and deal with the dependencies yourself.
