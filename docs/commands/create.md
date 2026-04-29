# create

Create an MPQ archive from a target directory or a single file.

## Create an MPQ archive from a target directory

Create an MPQ file from a target directory. Automatically adds `(listfile)` to the archive, and will skip this file if it exists in the target directory.

```bash
$ mpqcli create <target_directory>
```

The default mode of operation for the `create` subcommand is to take everything from the "target" directory (and below) and recursively add it to the archive. The directory structure is retained. Windows-style backslash path separators are used (`\`), as per the observed behavior in most MPQ archives.

## Create an MPQ archive for a specific game

Target a specific game version by using the `-g` or `--game` argument. This will automatically set the correct archive format version and settings, although they can be overridden.

```bash
$ mpqcli create -g starcraft <target_directory>
$ mpqcli create --game wow-wotlk \
    --sector-size 16384 \
    --version 3 <target_directory>
```

## Create an MPQ archive from a single file

Create an MPQ file from a single file.

```bash
$ mpqcli create --game diablo2 <target_file>
```

This will put the given file in the root of the MPQ archive. By optionally providing a path in the `--name-in-archive` parameter, the name that the file has in the MPQ archive can be changed, and it can be put in a directory.

## Create and sign an MPQ archive

Use the `-s` or `--sign` argument to cryptographically sign an MPQ archive with the Blizzard weak signature.

```bash
$ mpqcli create --version 1 --sign <target_directory>
```

## Create an MPQ archive with a given locale

Use the `--locale` argument to specify the locale that all added files will have in the archive. Note that subsequent added files will have the default locale unless the `--locale` argument is specified again.

```bash
$ mpqcli create <target_directory> --locale koKR
```
