# wow-mpq-parser

Collection of scripts which leverages the StormLib library to read, extract, and create MPQ files for Vanilla (1.12) World of Warcraft

## Quickstart

TODO

## Requirements

This project requires the [StormLib](https://github.com/ladislav-zezula/StormLib) library, which has a number of requirements, including:

- cmake
- g++
- zlib
- bzip2

These can generally be installed using the package manager on your system. For a Debian-based system, the following `apt` command should install all requirements:

```
sudo apt install -y cmake g++ zlib1g-dev bzip2
```

From here, you can download, build and install StormLib:

```
git clone https://github.com/ladislav-zezula/StormLib.git
cd StormLib
cmake CMakeLists.txt
make
make install
```

This will install the following files:

```
/usr/local/lib/libstorm.a
/usr/local/include/StormLib.h
/usr/local/include/StormPort.h
```
