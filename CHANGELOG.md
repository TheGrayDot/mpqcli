# Changelog

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
