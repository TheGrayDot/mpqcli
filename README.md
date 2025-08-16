# mpqcli

![Build Status](https://img.shields.io/github/actions/workflow/status/TheGrayDot/mpqcli/tag.yml?style=flat) ![Test Status](https://img.shields.io/github/actions/workflow/status/TheGrayDot/mpqcli/tag.yml?style=flat&label=test)

![Release Version](https://img.shields.io/github/v/release/TheGrayDot/mpqcli?style=flat)

![Release downloads](https://img.shields.io/github/downloads/thegraydot/mpqcli/total?label=Release%20downloads) ![Container downloads](https://img.shields.io/badge/Container_downloads-6-green)

A command line tool to create, add, remove, list, extract, read, and verify MPQ archives using the [StormLib library](https://github.com/ladislav-zezula/StormLib).

> ⚠️ **Warning:** This project is under active development and will change functionality between released versions until version 1.0.0.

## Overview

**This is a command-line tool, designed for automation and built with the Unix philosophy in mind.**  It is designed to work seamlessly with other command-line tools, supporting piping, redirection, and integration into shell scripts and workflows. For example:

- Run one command to create an MPQ archive from a directory of files
- Run one command to list all files in an MPQ archive
- Pipe the output to `grep` or other tools to search, filter, or process files
- Redirect output to files or other commands for further automation

**This project is primarily for older World of Warcraft MPQ archives**. This means it has been authored for MPQ archives versions 1 and 2, which include the following World of Warcraft (WoW) versions: Vanilla (1.12.1), TBC (2.4.3), and WoTLK (3.3.5). It has only been tested on WoW MPQ archives/patches that use MPQ versions 1 or 2. No testing has been performed on other MPQ versions or archives from other games. However, the tool will most likely work on other MPQ archive versions, as the underlying Stormlib library supports all MPQ archive versions.

If you require an MPQ tool with a graphical interface (GUI) and explicit support for more MPQ archive versions - I would recommend using [Ladik's MPQ Editor](http://www.zezula.net/en/mpq/download.html).

## Download

### Precompiled Binaries

Pre-built binaries are available for Linux and Windows.

Linux/WSL:

```
curl -fsSL https://raw.githubusercontent.com/thegraydot/mpqcli/main/scripts/install.sh | bash

Microsoft Windows:

```
irm https://raw.githubusercontent.com/thegraydot/mpqcli/main/scripts/install.ps1 | iex
```

Check the [latest release with binaries](https://github.com/TheGrayDot/mpqcli/releases).

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

The `mpqcli` program has the following subcommands:

- `version`: Print the tool version number
- `about`: Print information about the tool
- `info`: Print information about an MPQ archive properties
- `create`: Create an MPQ archive from a target directory
- `add`: Add a file to an existing MPQ archive (not implemented)
- `remove`: Remove a file from an existing MPQ archive (not implemented)
- `list`: List files in a target MPQ archive
- `extract`: Extract one/all files from a target MPQ archive
- `read`: Read a specific file to stdout
- `verify`: Verify a target MPQ archive signature

## Command Examples

All of the examples use the MPQ archive file, named `wow-patch.mpq`, from a Vanilla World of Warcraft patch file, named `WoW-1.10.0-to-1.10.1-enGB-patch.zip`. If you want to replicate these examples, you can [download the `wow-patch.mpq` file](https://archive.org/download/World_of_Warcraft_Client_and_Installation_Archive/Patches/1.x/WoW-1.10.0-to-1.10.1-enGB-patch.zip/wow-patch.mpq) from the Internet Archive.

### Print information about an MPQ archive

The `info` subcommand prints a list of useful information (property keys and values) of an MPQ archive.

```
mpqcli info wow-patch.mpq
Archive size: 1798918
File count: 65
Format version: 1
Header offset: 0
Header size: 32
Max files: 128
Signature type: Weak
```

### Print one specific MPQ archive property

The `info` subcommand supports the following properties:

```
format-version
header-offset
header-size
archive-size
file-count
signature-type
```

You can use the `-p` or `--property` argument with the `info` subcommand to print just the value of a specific property. This can be useful for automation, for example, to determine the signature type of a directory of MPQ archives.

```
mpqcli info -p file-count wow-patch.mpq
65
```

### Create an MPQ archive from a target directory

Create an MPQ file from a target directory. Automatically adds `(listfile)` to the archive, and will skip this file if it exists in the target directory.

```
mpqcli create <target_directory>
```

The default mode of operation for the `create` subcommand is to take everything from the "target" directory (and below) and recursively add it to the archive. The directory structure is retained. Windows-style backslash path separators are used (`\`), as per the observed behavior in most MPQ archives.

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

Add a local file to an already existing MPQ archive.

```
echo "For The Horde!" > fth.txt
mpqcli add fth.txt wow-patch.mpq
```

### Remove a file from an existing archive

Remove a file from an existing MPQ archive.

```
mpqcli remove fth.txt wow-patch.mpq
```

### List all files in an MPQ archive

Pretty simple, list files in an MPQ archive. Useful to "pipe" to other tools, such as `grep` (see below for examples).

```
mpqcli list wow-patch.mpq
BM_COKETENT01.BLP
Blizzard_CraftUI.xml
CreatureSoundData.dbc
...
Blizzard_CraftUI.lua
30ee7bd3959906e358eff01332cf045e.blp
realmlist.wtf
```

### List all files with detailed output

Similar to the `ls` command with the `-l` and `-a` options, additional detailed information can be included with the `list` subcommand. The `-a` option includes printing "special" files used in MPQ archives including: `(listfile)`, `(attributes)` and `(signature)`.

```
mpqcli list -d -a wow-patch.mpq 
      88604  enUS  Mar 29 2006 12:02 BM_COKETENT01.BLP
        243  enUS  Apr  5 2006 07:28 Blizzard_CraftUI.xml
        388  enUS  Mar 30 2006 05:32 CreatureSoundData.dbc
        ...
        184  enUS  Apr  5 2006 07:28 Blizzard_CraftUI.lua
      44900  enUS  Mar 29 2006 12:01 30ee7bd3959906e358eff01332cf045e.blp
         68  enUS  Apr  7 2006 10:58 realmlist.wtf
```

### List all files with an external listfile

Older MPQ archives do not contain (complete) file paths of their content. By providing an external listfile that lists the content of the MPQ archive, the listed files will have the correct paths. Listfiles can be downloaded on [Ladislav Zezula's site](http://www.zezula.net/en/mpq/download.html).

```
mpqcli list -l /path/to/listfile <target_mpq_archive>
```

### Extract all files from an MPQ archive

The output will be saved in a folder with the same name as the target MPQ file, without the extension.

```
mpqcli extract wow-patch.mpq
[+] Extracted: BM_COKETENT01.BLP
[+] Extracted: Blizzard_CraftUI.xml
[+] Extracted: CreatureSoundData.dbc
...
[+] Extracted: Blizzard_CraftUI.lua
[+] Extracted: 30ee7bd3959906e358eff01332cf045e.blp
[+] Extracted: realmlist.wtf
```

### Extract all files to a target directory

Extract files to a specific target directory, which, will be created if it doesn't already exist. In this example, `patch-1.10` is the user-specified output directory.

```
mpqcli extract -o patch-1.10 wow-patch.mpq
```

### Extract all files with an external listfile

Older MPQ archives do not contain (complete) file paths of their content. By providing an external listfile that lists the content of the MPQ archive, the extracted files will have the correct names and paths. Listfiles can be downloaded on [Ladislav Zezula's site](http://www.zezula.net/en/mpq/download.html).

```
mpqcli extract -l path/to/listfile <target_mpq_archive>
```

### Extract one specific file

Extract a single file using the `-f` option. If the target file in the MPQ archive is nested (in a directory) you need to include the full path. Similar to the examples above, you can use the `-o` argument to specify the output directory.

```
mpqcli extract -f "Documentation\Layout\Greeting.html" "World of Warcraft_1.12.1.5875/Data/base.MPQ"
```

### Read a specific file from an MPQ archive

Read the `patch.cmd` file from an MPQ archive and print the file contents to stdout. Even though the subcommand always outputs bytes, plaintext files will be human-readable.

```
mpqcli read patch.cmd wow-patch.mpq
* set the product patch name
PatchVersion This patch upgrades World of Warcraft from version 1.10.0.5195 to version 1.10.1.5230.
* make sure that we don't patch version 1.10.1.5230 or greater
FileVersionEqualTo    "$(InstallPath)\WoW.exe"    1.10.0.5195   0xffff.0xffff.0xffff.0xffff
* ProductVersionLessThan "$(InstallPath)\WoW.exe"    0.0.0.256  0.0.0.0xffff
Language enGB
SetLauncher "$(InstallPath)\WoW.exe"
* SetUninstall $(WinDir)\WoWUnin.dat
Self "World of Warcraft"
WoWPatchIndex 2
PatchSize 1992294400
```

The tool will always print output in bytes and is designed to be "piped" or redirected. For example, redirect the binary `WoW.exe` file from a WoW patch to a local file:

```
mpqcli read "WoW.exe" wow-patch.mpq > WoW.exe
```

Another example, piping the bytes to the `xxd` tool.

```
mpqcli read "WoW.exe" wow-patch.mpq | xxd
00000000: 1800 0404 de2a a5da 3240 4500 3250 4500  .....*..2@E.2PE.
00000010: 69ac 2703 0859 c601 0a7a 4500 8942 5344  i.'..Y...zE..BSD
00000020: 4946 4634 30a0 2905 8203 f244 0482 3250  IFF40.)....D..2P
00000030: 4504 8080 0280 9002 8098 0281 d801 0585  E...............
```

### Verify an MPQ archive

Check the digital signature of an MPQ archive, by verifying the signature in the archive with a collection of known Blizzard public keys (bundled in Stormlib library). The tool will print if verification is successful or fails, as well as return `0` for success and any other value for failure.

```
mpqcli verify wow-patch.mpq
[+] Verify success
```

If verification passes, a zero (`0`) exit status will be returned. This can be helpful to verify a large number of MPQ archives without the need to review the status message that is printed out.

```
echo $?
0
```

### Verify an MPQ archive and print the digital signature

Check the digital signature of an MPQ archive, by verifying the signature in the archive and also printing the digital signature value in bytes using the `-p` or `--print` argument.

```
mpqcli verify -p wow-patch.mpq > signature
```

## Advanced Command Examples

### Search and extract files on Linux

The `mpqcli` tool has no native search feature - instead, it is designed to be integrated with other, external operating system tools. For example, `mpqcli list` can be "piped" to `grep` in Linux or `Select-String` in Windows Powershell to perform searching.

The following command lists all files in an MPQ archive, and each filename is filtered using `grep` - selecting files with `exe` in their name, which is then passed back to `mpqcli extract`. The result: search and extract all `exe` files.

```
mpqcli list wow-patch.mpq | grep ".exe" | xargs -I@ mpqcli extract -f "@" wow-patch.mpq
[+] Extracted: Launcher.exe
[+] Extracted: BackgroundDownloader.exe
[+] Extracted: WoW.exe
[+] Extracted: BNUpdate.exe
[+] Extracted: Repair.exe
[+] Extracted: WowError.exe
```

### Search and extract files on Windows

The following command lists all files in an MPQ archive, and each filename is filtered using `grep` - selecting files with `exe` in their name, which is then passed back to `mpqcli extract`. The result: search and extract all `exe` files.

```
mpqcli.exe list wow-patch.mpq | Select-String -Pattern ".exe" | ForEach-Object { mpqcli.exe extract -f $_ wow-patch.mpq }
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

This project requires the [StormLib](https://github.com/ladislav-zezula/StormLib) library. Many thanks to [Ladislav Zezula](https://github.com/ladislav-zezula) for authoring such a good library and releasing the code under an open-source license. The StormLib library has a number of requirements. However, the build method specifies using the libraries bundled with StormLib.

### CLI11

This project also uses the [CLI11](https://github.com/CLIUtils/CLI11) command line parser for C++11. It provides simple and easy-to-use CLI arguments.

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
