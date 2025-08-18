# Changelog

## 0.9.1 - 2025-08-19

### Fixed

- Bug in Dockerfiles resulting in missing commit hash from version

## 0.9.0 - 2025-08-19

### Added

- Linux binaries for amd64 and arm64
- Linux binaries built with musl and glibc

### Updated

- Install script for Linux to provided better binary download options

### Changed

- Linux binary release naming convention

## 0.8.1 - 2025-08-15

### Added

- Added print raw bytes for `read` and `verify` subcommands

### Fixed

- Tests for `read` and `verify` subcommands

### Removed

- Ability to print hex or plaintext in `read` and `verify` subcommands
- Removed messages when printing signature value

## 0.8.0 - 2025-08-11

### Added

- Added `add`/`remove` subcommands working
- Added tests for `add`/`remove` subcommands
- Added `max-files` property to `info` subcommand

### Changed

- Change for default maximum file count when creating MPQ archives

## 0.7.0 - 2025-08-09

### Added

- Added detailed (long) listing

### Fixed

- Updated create subcommand to always include attributes
- Updated Info function to support archive/file property fetching

## 0.6.2 - 2025-07-11

### Added

- Added tests for `extract` subcommand
- Added dependabot configuration for Python and Docker

### Fixed

- Fixed deprecated Windows Action running to Server 2022

## 0.6.1 - 2025-06-27

### Fixed

- Updated readme and help menu to match application functionality

## 0.6.0 - 2025-06-02

### Added

- Added the `read` subcommand
- Added tests for `read` subcommand
- Added new helper functions to print ASCII or hexidecimal output

### Fixed

- Fixed line endings in Windows pytest cases

## 0.5.0 - 2025-05-05

### Added

- Added the `verify` subcommand
- Included argument to print digital signature
- Added ability to sign archive when creating MPQ
- Added tests for `verify` subcommand

## 0.4.4 - 2025-05-05

### Fixed

- Refactored `create` subcommand tests

## 0.4.3 - 2025-05-01

### Added

- Refactored `info` subcommand
- Added test for `info` subcommand

### Fixed

- Updated tests to be multi-platform (Windows)

### Removed

- Removed exe patch split functionality

## 0.4.2 - 2025-04-22

### Added

- Added Dockerfile
- Added Docker build and release workflows
- Added `add` and `remove` subcommand placeholders
- Added `about` subcommand

## 0.4.1 - 2025-04-22

### Added

- Added Python testing environment
- Added tests for `list` subcommand
- Added test workflow

## 0.4.0 - 2025-04-12

### Added

- Added Windows binary build to releases

## 0.3.2 - 2025-03-29

### Added

- External listfile support for `list` and `extract` by @sjoblomj
- Added functions for add and remove files

### Fixed

- Fixed path extraction on Linux to create nested folder structure

## 0.3.1 - 2025-03-03

### Fixed

- Fixed path extraction on Linux to create nested folder structure

## 0.3.0 - 2025-02-24

### Added

- Added `create` subcommand
- Added download information to readme

### Fixed

- Improved Linux binary compatibility

## 0.2.3 - 2025-02-01

### Fixed

- Improved Linux binary compatibility

## 0.2.2 - 2025-02-01

### Added

- Added the `extract`, `verify` and `list` subcommands
- Added `release.yml` workflow for Linux binary
