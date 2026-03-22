# Contributing

Contributions are welcome. Please read the guidelines below before opening a pull request.

## Before You Start

If you are unsure whether a feature fits the project, or whether an existing tool could already be combined with `mpqcli` to achieve the same result, open an issue first. This avoids wasted effort and keeps the project focused.

**mpqcli follows the Unix philosophy.** The tool is designed to do one thing well and to compose with other tools via pipes and redirection. If you find yourself wanting to add functionality that could be handled by a separate tool — for example, sorting the output of `list` — the right answer is usually to pipe the output to that tool rather than adding it here.

## Requirements for a Pull Request

### 1. Builds on your platform

Make sure the project builds cleanly on your development machine before opening a PR:

```
make build_linux   # Linux
make build_windows # Windows
```

A PR automatically triggers the CI build workflow, which compiles and tests across all supported Linux targets (AMD64 and ARM64, glibc and musl). You are not expected to reproduce all of those locally.

### 2. Tests pass

Run the test suite before submitting:

```
make test_create_venv  # first-time setup only
make test_mpqcli
```

All tests must pass without errors.

### 3. New features should include tests

If your change adds or modifies user-facing functionality — such as a new subcommand flag or a change in output format — please include a corresponding test in the `test/` directory. The existing test files (`test_list.py`, `test_add.py`, etc.) are good references for the test style and fixtures used.

### 4. Match the existing code style

There is no enforced formatter. Write C++ that looks consistent with the surrounding code, and Python tests that follow the style of the existing test files.

## Workflow Summary

1. Fork the repository and create a branch for your change
2. Make your changes and verify they build and all tests pass
3. Open a pull request with a clear description of what was changed and why
