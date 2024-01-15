#!/bin/bash


PARENT_DIR="$(dirname "$(dirname "$(readlink -fm "$0")")")"
TARGET_FI="$PARENT_DIR/CMakeLists.txt"
MAJOR=$(grep "VERSION_MAJOR" "$TARGET_FI" | cut -d' ' -f2 | cut -d')' -f1 | head -1)
MINOR=$(grep "VERSION_MINOR" "$TARGET_FI" | cut -d' ' -f2 | cut -d')' -f1 | head -1)
PATCH=$(grep "VERSION_PATCH" "$TARGET_FI" | cut -d' ' -f2 | cut -d')' -f1 | head -1)
PROJECT_VERSION="v$MAJOR.$MINOR.$PATCH"

echo "[*] Current version: $PROJECT_VERSION"

read -rp "[*] Tag and Release? (y/N) " yn
case $yn in
	y ) git tag "$PROJECT_VERSION";
        git push --tags;
        exit 0;;
	n ) echo "[*] Exiting...";
		exit 0;;
	* ) echo "[*] Invalid response... Exiting";
        exit 1;;
esac
