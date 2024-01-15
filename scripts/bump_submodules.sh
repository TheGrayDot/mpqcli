#!/bin/bash


PARENT_DIR="$(dirname "$(dirname "$(readlink -fm "$0")")")"
cd "$PARENT_DIR" || exit 1

read -rp "[*] Bump StormLib? (y/N) " yn
case $yn in
	y ) git submodule update --init --remote extern/StormLib;;
	n ) echo "[*] Skipping...";;
	* ) echo "[*] Invalid response... Skipping";;
esac

read -rp "[*] Bump CLI11? (y/N) " yn
case $yn in
	y ) git submodule update --init --remote extern/CLI11;;
	n ) echo "[*] Skipping...";;
	* ) echo "[*] Invalid response... Skipping";;
esac
