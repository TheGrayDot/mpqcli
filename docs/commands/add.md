# add

Add a file to an existing MPQ archive.

## Add a file to an existing archive

Add a local file to an already existing MPQ archive.

```bash
$ echo "For The Horde" > fth.txt
$ mpqcli add fth.txt wow-patch.mpq
[+] Adding file: fth.txt
```

Alternatively, you can add a file under a specific file name using the `-f` or `--filename-in-archive` argument.

```bash
$ echo "For The Alliance" > fta.txt
$ mpqcli add fta.txt wow-patch.mpq --filename-in-archive "alliance.txt"
[+] Adding file: alliance.txt
```

Alternatively, you can add a file to a specific subdirectory using the `-d` or `--directory-in-archive` argument.

```bash
$ echo "For The Swarm" > fts.txt
$ mpqcli add fts.txt wow-patch.mpq --directory-in-archive texts
[+] Adding file: texts\fts.txt
```

Alternatively, you can add a file under a specific directory and filename using the `-p` or `--path` argument.

```bash
$ echo "For The Swarm" > fts.txt
$ mpqcli add fts.txt wow-patch.mpq --path "texts\swarm.txt"
[+] Adding file: texts\swarm.txt
```

To overwrite a file in an MPQ archive, set the `-w` or `--overwrite` flag:

```bash
$ echo "For The Horde" > allegiance.txt
$ mpqcli add allegiance.txt wow-patch.mpq
[+] Adding file: allegiance.txt
$ echo "For The Alliance" > allegiance.txt
$ mpqcli add allegiance.txt wow-patch.mpq
[!] File already exists in MPQ archive: allegiance.txt - Skipping...
$ mpqcli add allegiance.txt wow-patch.mpq --overwrite
[+] File already exists in MPQ archive: allegiance.txt - Overwriting...
[+] Adding file: allegiance.txt
```

## Add a file to an MPQ archive with a given locale

Use the `--locale` argument to specify the locale that the added file will have in the archive. Note that subsequent added files will have the default locale unless the `--locale` argument is specified again.

```bash
$ mpqcli add allianz.txt wow-patch.mpq --locale deDE
[+] Adding file for locale deDE: allianz.txt
```

## Add a file with game-specific properties

Target a specific game version by using the `-g` or `--game` argument. This will automatically set the correct encryption rules and MPQ flags, although they can be overridden.

```bash
$ mpqcli add khwhat1.wav archive.mpq --game wc2
[+] Adding file: khwhat1.wav
```
