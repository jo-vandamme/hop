## Hop

Simple Monte-Carlo path tracer in C++.
This is a work in progress, done on my spare time.

[![Build status](https://travis-ci.org/jo-va/hop.svg?branch=master)](https://travis-ci.org/jo-va/hop)

## Features
- OBJ model loading
- Instancing
- Data driven configuration via Lua
- Interactive rendering
- Depth of field

Since I am developping on Linux, the code is targeted to Linux platforms for now, but supporting Windows/Mac OS should not be too difficult.

## Compiling and running
Requires cmake and OpenMP for multithreading.

Download & Compile:
```
$ git clone --recursive https://github.com/jo-va/hop.git
$ cd hop
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Run:
```
$ ./hop scene.lua
```

## Sample Images

