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



g++ -o storm storm.cpp -I/usr/local/include/ -lstorm -lz -lbz2

g++ -o storm storm.cpp -I/usr/local/include/ -lstorm -lz -lbz2; ./storm base.MPQ

## Patch Notes

difference between enus and engb are locales

 2008  cd wow-1.12.0.5595-to-1.12.1.5875-enus-patch/
 2009  md5sum * > ../wow-1.12.0.5595-to-1.12.1.5875-enus-patch.md5 

 2011  cd wow-1.12.0.5595-to-1.12.1.5875-engb-patch/
 2013  md5sum -c ../wow-1.12.0.5595-to-1.12.1.5875-enus-patch.md5 

(attributes): FAILED
FRAMEXML.TOC.SIG: FAILED
GLUEXML.TOC.SIG: FAILED
hdfiles.lst: FAILED
optional.lst: FAILED
patch.cmd: FAILED
Patch.html: FAILED
Patch.txt: FAILED
realmlist.wtf: FAILED
(signature): FAILED
wowtest.lst: FAILED
md5sum: WARNING: 11 computed checksums did NOT match

https://www.bluetracker.gg/wow/topic/eu-en/15732227-please-post-mirror-links-for-1120-1121/
^^ Official verified hashes
WoW-1.12.0.5595-to-1.12.1.5875-enGB-patch.exe - d69c9cab51482f3175abec4b204fe78b

./archive/WoW-1.12.0.5595-to-1.12.1.5875-enGB-patch.exe
^^ This file has a hash match

So - I converted exe to mpq for enus and engb
then extracted all file
then hash compared them
The results are as above (11 failed files)
However, this does not check the data in the preceeding exe
