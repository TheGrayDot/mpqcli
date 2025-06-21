# mpqcli

![Build Status](https://img.shields.io/github/actions/workflow/status/TheGrayDot/mpqcli/tag.yml?style=flat) ![Test Status](https://img.shields.io/github/actions/workflow/status/TheGrayDot/mpqcli/tag.yml?style=flat&label=test)

![Release](https://img.shields.io/github/v/release/TheGrayDot/mpqcli?style=flat) ![Downloads](https://img.shields.io/github/downloads/TheGrayDot/mpqcli/total?style=flat)

A command line tool to create, add, remove, list, extract, read, and verify MPQ archives using the [StormLib library](https://github.com/ladislav-zezula/StormLib).

> ⚠️ **Warning:** This project is under active development and will change functionality between released versions until version 1.0.0.

## Overview

**This is a command line tool, designed for automation**. For example:

- Run one command to create an MPQ archive from a directory of files
- Run one command to list all files in an MPQ archive
- Run one command to search and extract files from an MPQ archive

**This project is for primarily for older World of Warcraft MPQ archives**. This means is has been authored for MPQ files versions 1 and 2, which includes the following World of Warcraft (WoW) versions: Vanilla (1.12.1), TBC (2.4.3) and WoTLK (3.3.5). It has only been tested on WoW MPQ archives/patches which use MPQ versions 1 or 2. No testing has been performed on other MPQ versions or archives from other games. However, the tool will most likely work, as the underlying Stormlib library supports these other versions.

If you require an MPQ tool with a graphical interface (GUI) and explicit support for more MPQ archive versions - I would recommend using [Ladik's MPQ Editor](http://www.zezula.net/en/mpq/download.html).

## Download

### Precompiled Binaries

Check the [latest release with binaries](https://github.com/TheGrayDot/mpqcli/releases).

### Latest Precompiled Binary on Linux

Download latest release on Linux:

```
TAG=$(curl -s https://api.github.com/repos/TheGrayDot/mpqcli/releases/latest | grep -oP '"tag_name": "\K(.*)(?=")')
curl -L -o mpqcli https://github.com/TheGrayDot/mpqcli/releases/download/$TAG/mpqcli-linux
chmod u+x mpqcli
./mpqcli version
```

Add the `mpqcli` binary to your path:

```
mv mpqcli /usr/local/bin/
```

### Latest Precompiled Binary on Windows

Download the latest release on Windows:

```
$TAG = (Invoke-RestMethod -Uri "https://api.github.com/repos/TheGrayDot/mpqcli/releases/latest").tag_name
Invoke-WebRequest -Uri "https://github.com/TheGrayDot/mpqcli/releases/download/$TAG/mpqcli-windows.exe" -OutFile "mpqcli.exe"
.\mpqcli.exe version
```

### Docker Image

The Docker image for `mpqcli` is hosted on [GitHub Container Registry (GHCR)](https://ghcr.io). It provides a lightweight and portable way to use `mpqcli` without needing to build or download a binary.

To download the latest version of the `mpqcli` Docker image, run:

```
docker pull ghcr.io/thegraydot/mpqcli:latest
```

You can run `mpqcli` commands directly using the Docker container. For example:

```
docker run ghcr.io/thegraydot/mpqcli:latest version
```

To use local files in the container, mount a directory from your host system. In the following example, the `-v` argument is used to mount the present working directory to `/data` directory in the container. Then the `mpqcli` container runs the `list` subcommand with `/data/example.mpq` as the target MPQ archive.

```
docker run -v $(pwd):/data ghcr.io/thegraydot/mpqcli:latest list /data/example.mpq
```

## Subcommands

The `mpqcli` program has the follwoing subcommands:

- `version`: Print the tool version number
- `about`: Print information about the tool
- `create`: Create an MPQ archive from a target directory
- `add`: Add a file to an existing MPQ archive (not implemented)
- `remove`: Remove a file from an existing MPQ archive (not implemented)
- `list`: List files in a target MPQ archive
- `extract`: Extract one/all files from a target MPQ archive
- `read`: Read a specific file to stdout
- `verify`: Verify a target MPQ archive signature

## Command Examples

### Create an MPQ archive from a target directory

Create an MPQ file from a target directory. Automatically adds `(listfile)` to the archive, and will skip this file if it exists in the target directory.

```
mpqcli create <target_directory>
```

### Create an MPQ archive using a specific version

Support for creating an MPQ archive version 1 or version 2 by using the `-v` or `--version` argument.

```
mpqcli create -v 1 <target_directory>
mpqcli create --version 2 <target_directory>
```

### Create and sign an MPQ archive

Use the `-s` or `--sign` argument to cryptographically sign an MPQ archive with the Blizzard weak signature.

```
mpqcli create --version 1 --sign <target_directory>
```

### Add a file to an existing archive

The `add` subcommand has not yet been added.

### Remove a file from an existing archive

The `remove` subcommand has not yet been added.

### List all files in an MPQ archive

Pretty simple, list files in an MPQ archive. Useful to "pipe" to other tools, such as `grep` (see below for examples).

```
mpqcli list <target_mpq_file>
```

### List all files with an external listfile

Older MPQ archives do not contain (complete) file paths of their content. By providing an external listfile that lists the content of the MPQ archive, the listed files will have the correct paths. Listfiles can be downloaded on [Ladislav Zezula's site](http://www.zezula.net/en/mpq/download.html).

```
mpqcli list -l /path/to/listfile <target_mpq_file>
```

### Extract all files from an MPQ archive

The output will be saved in a folder with the same name as the target MPQ file, without the extension.

```
mpqcli extract <target_mpq_file>
```

### Extract all files to a target directory

Extract files to a specific target directory, which, will be created if it doesn't already exist.

```
mpqcli extract -o /path/to/target/directory <target_mpq_file>
```

### Extract all files with an external listfile

Older MPQ archives do not contain (complete) file paths of their content. By providing an external listfile that lists the content of the MPQ archive, the extracted files will have the correct names and paths. Listfiles can be downloaded on [Ladislav Zezula's site](http://www.zezula.net/en/mpq/download.html).

```
mpqcli extract -l /path/to/listfile <target_mpq_file>
```

### Extract one specific file

Extract a single file using the `-f` option. If the target file in the MPQ archive is nested (in a directory) you need to include the full path. Similar to examples above, you can use the `-o` argument to specify the output directory.

```
mpqcli extract -f "InstallCD\Unpack\InstallLogTemplate\BaseHeader.html" <target_mpq_file>
```

### Read a specific file from an MPQ archive

TODO

### Verify an MPQ archive

TODO

## Advanced Command Examples

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

## Building

### Requirements

- cmake
- C++ 17 compiler
- StormLib (provided as Git submodule)
- CLI11 (provided as GitSubmodule)

### Linux

```
git clone --recursive https://github.com/TheGrayDot/mpqcli.git
cd mpqcli
cmake -B build
cmake --build build
```

The `mpqcli` binary will be available in: `./build/bin/mpqcli`

### Windows

```
git clone --recursive https://github.com/TheGrayDot/mpqcli.git
cd mpqcli
cmake -B build
cmake --build build --config Release
```

The `mpqcli.exe` binary will be available in: `.\build\bin\Release\mpqcli.exe`

## Dependencies

### StormLib

This project requires the [StormLib](https://github.com/ladislav-zezula/StormLib) library. Many thanks to [Ladislav Zezula](https://github.com/ladislav-zezula) for authoring such a good library and releasing the code under an open source licence. The StormLib library has a number of requirements. However, the build method specifies using the libraries bundled with StormLib.

### CLI11

This project also uses the [CLI11](https://github.com/CLIUtils/CLI11) command line parser for C++11. It provides a simple and easy to use/implement CLI arguments.

## Tests

This project implements End-to-end (E2E) testing, sometimes referred to as system testing or integration testing. This methodology is used because it verifies application functionality by simulating actual usage by an end user. Testing includes creating a variety of MPQ archives, as well as dynamically  downloading some small (~1-5MB) MPQ archives from the Internet Archive. The Python programming language coupled with the [pytest framework](https://github.com/pytest-dev/pytest) is used to implement testing, mainly due to ease of implementation.

To configure the testing environment you will need Python installed, as well as the required `pytest` package. On a Debian-based Linux system, the following will configure the environment: 

```
sudo apt install python3-venv python3-pip
python3 -m venv test/venv
source test/venv/bin/activate
pip3 install -r test/requirements.txt
```

Then you can run the tests using:

```
python3 -m pytest test -s
```
