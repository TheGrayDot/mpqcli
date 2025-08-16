#!/bin/bash


PACKAGE_URL="https://github.com/TheGrayDot/mpqcli/pkgs/container/mpqcli"

page_content=$(curl -s "$PACKAGE_URL")

# Debugging
# echo "$page_content" | grep -i "total downloads" -A5 -B5

downloads=$(echo "$page_content" \
    | grep -A2 "Total downloads" \
    | grep -o '<h3 title="[0-9]*">[0-9]*</h3>' \
    | grep -o 'title="[0-9]*"' \
    | grep -o '[0-9]*' \
    | head -1)

echo "![Package downloads](https://img.shields.io/badge/package_downloads-$downloads-green)"
