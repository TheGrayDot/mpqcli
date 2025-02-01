# mpqcli

![Build Status](https://img.shields.io/github/actions/workflow/status/TheGrayDot/mpqcli/build.yml?branch=main&style=flat)

A command line tool to read, extract, search, create and verify MPQ files using the StormLib library

## Overview

**This is a command line tool, designed for automation**. For example, run one command to create an MPQ file from a directory of files. If you require an MPQ tool with a graphical interface (GUI) - I would recommend using [Ladik's MPQ Editor](http://www.zezula.net/en/mpq/download.html).

**This project is for original World of Warcraft MPQ archives**. This means is has been primarily authored for MPQ files used in the following World of Warcraft (WoW) versions: Vanilla (1.12.1), TBC (2.4.3) and WoTLK (3.3.5). It has only been tested on WoW MPQ archives/patches which use MPQ versions 1 or 2. No testing has been performed on other MPQ versions or archives from other games.

## Download

Download latest release on Linux:

```
TAG=$(curl -s https://api.github.com/repos/TheGrayDot/mpqcli/releases/latest | grep -oP '"tag_name": "\K(.*)(?=")')
curl -L -o mpqcli-linux https://github.com/TheGrayDot/mpqcli/releases/download/$TAG/mpqcli-linux
chmod u+x mpqcli-linux
./mpqcli-linux --help
```

## Building

### Requirements

- cmake
- C++ 17 compiler
- StormLib (provided as Git submodule)
- CLI11 (provided as GitSubmodule)

### Quickstart

```
git clone --recursive https://github.com/TheGrayDot/mpqcli.git
cd mpqcli
cmake -B build
cmake --build build
```

The `mpqcli` binary will be available in: `./build/bin/mpqcli`

## Subcommands

The `mpqcli` program has the follwoing subcommands:

- `version`: Print the tool version number
- `info`: Print info about a target MPQ archive
- `extract`: Extract one/all files from a target MPQ archive
- `create`: Create an MPQ archive from a target directory (not implemented))
- `add`: Add a file to an existing MPQ archive (not implemented)
- `list`: List files in a target MPQ archive
- `verify`: Verify a target MPQ archive
- `patch`: Functionality to handle self-executable patch files

## Example Commands

### Extract all files

The output will be saved in a folder with the same name as the target MPQ file, without the extension.

```
mpqcli extract <target_mpq_file>
```

### Extract all files to a target directory

Extract files to a specific target directory - which will be created if it doesn't already exist.

```
mpqcli extract -o /path/to/target/directory <target_mpq_file>
```

### Extract one specific file

Extract a single file using the `-f` option. If the target file in the MPQ archive is nested (in a directory) you need to include the full path. Similar to examples above, you can use the `-o` argument to specify the output directory.

```
mpqcli extract -f "InstallCD\Unpack\InstallLogTemplate\BaseHeader.html" <target_mpq_file>
```

### List all files

Pretty simple, list files in an MPQ archive. Useful to "pipe" to other tools, such as `grep` (see below for examples).

```
mpqcli list <target_mpq_file>
```

### Search and extract files on Linux

The `mpqcli` tool has no native search feature - instead it is designed to be integrated with other, extenal operating system tools. For example, `mpqcli list` can be "piped" to `grep` in Linux or `Select-String` in Windows Powershell to perform searching.

The following command lists all files in an MPQ archive, and each filename is filtered using `grep` - selecting files with `exe` in their name, which is then passed back to `mpqcli extract`. The result: search and extract all `exe` files.

```
mpqcli list <target_mpq_file> | grep ".exe" | xargs -I@ mpqcli extract -f "@" <target_mpq_file>
```

### Search and extract files on Windows

The following command lists all files in an MPQ archive, and each filename is filtered using `grep` - selecting files with `exe` in their name, which is then passed back to `mpqcli extract`. The result: search and extract all `exe` files.

```
mpqcli.exe list <target_mpq_file> | Select-String -Pattern ".exe" | ForEach-Object { mpqcli.exe extract -f $_ <target_mpq_file> }
```

### Create archive from a target directory

The `create` subcommand has not yet been added.

### Add a file to an existing archive

The `add` subcommand has not yet been added.

### Verify Archive

TODO.

### Extract an MPQ archive from a self-executable patch

Extract an MPQ archive from a self-executable patch file (`.exe`), for example, a WoW executable patch (e.g., `WoW-1.12.0.5595-to-1.12.1.5875-enUS-patch.exe`). 

```
mpqcli patch <target_exe_file>
```

### Extract an MPQ archive and binary data from a self-executable patch

Extract an MPQ archive from a patch file (e.g., `.exe`), and extract the preceeding data (as a `.bin` file). 

```
mpqcli patch -b <target_exe_file>
```

## Dependencies

### StormLib

This project requires the [StormLib](https://github.com/ladislav-zezula/StormLib) library. Many thanks to [Ladislav Zezula](https://github.com/ladislav-zezula) for authoring such a good library and releasing the code under an open source licence. The StormLib library has a number of requirements. However, the build method specifies using the libraries bundled with StormLib.

### CLI11

This project also uses the [CLI11](https://github.com/CLIUtils/CLI11) command line parser for C++11. It provides a simple and easy to use/implement CLI arguments.
