# mpqcli

![Project Status Badge](https://img.shields.io/badge/Status-Active%20development-red)

![C++ Standard Version](https://img.shields.io/badge/Version-11-blue.svg?style=flat&logo=c%2B%2B)

A command line tool to read, extract, search, create and verify MPQ file using the StormLib library

## Requirements

- cmake
- C++ 11 compiler
- StormLib (provided as Git submodule)

## Quickstart

```
git clone --recursive https://github.com/TheGrayDot/mpqcli.git
cd mpqcli && mkdir build && cd build
cmake ..
make
```

The `mpqcli` binary will be available in: `mpqcli/build/bin/mpqcli`

## Example Commands

The `mpqcli` program has a collection of subcommands including:

- `info`: Print info about a target MPQ file
- `extract`: Extract all files from a target MPQ file
- `create`: Create an MPQ file from a target directory
- `search`: Search files in a target MPQ file
- `verify`: Verify a target MPQ file
- `patch`: Functionality to handle EXE patch files

### Extract Files

Extract files from an MPQ archive. The output will be saved in a folder with the same name as the target MPQ file, without the extension.

```
mpqcli extract <target_mpq_file>
```

Extract files from an MPQ archive, specifying a target output directory - which must already exist.

```
mpqcli extract -o /path/to/existing/directory <target_mpq_file>
```

## StormLib

This project requires the [StormLib](https://github.com/ladislav-zezula/StormLib) library. Many thanks to [Ladislav Zezula](https://github.com/ladislav-zezula) for authoring such a good library and releasing the code under an open source licence. The StormLib library has a number of requirements. However, the build method specifies using the libraries bundled with StormLib.
