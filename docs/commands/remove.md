# remove

Remove a file from an existing MPQ archive.

## Remove a file from an existing archive

Remove a file from an existing MPQ archive.

```bash
$ mpqcli remove fth.txt wow-patch.mpq
[-] Removing file: fth.txt
```

## Remove a file from an MPQ archive with a given locale

Use the `--locale` argument to specify the locale of the file to be removed.

```bash
$ mpqcli remove alianza.txt wow-patch.mpq --locale esES
[-] Removing file for locale esES: alianza.txt
```
