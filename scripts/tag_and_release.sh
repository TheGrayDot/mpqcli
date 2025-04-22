#!/bin/bash


PARENT_DIR="$(dirname "$(dirname "$(readlink -fm "$0")")")"
TARGET_FI="$PARENT_DIR/CMakeLists.txt"

PROJECT_VERSION="v$(awk '/project\(.*VERSION/ {print $NF}' "$TARGET_FI" | tr -d ')')"

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
