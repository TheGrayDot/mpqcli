CMAKE_BUILD_TYPE := Release
BUILD_MPQCLI := ON
CLANG_VERSION := 18
VERSION := $(shell awk '/project\(MPQCLI VERSION/ {gsub(/\)/, "", $$3); print $$3}' CMakeLists.txt)
README := README.md
PACKAGE_URL := https://github.com/TheGrayDot/mpqcli/pkgs/container/mpqcli

GCC_INSTALL_DIR := $(shell dirname "$(shell gcc -print-libgcc-file-name)")

.PHONY: help \
	setup \
	build_linux build_windows build_clean build_lint_clean \
	docker_musl_build docker_musl_run docker_glibc_build docker_glibc_run \
	test_create_venv test_mpqcli test_clean test_lint \
	lint_format lint_format_fix lint_cpp lint \
	clean \
	bump_stormlib bump_cli11 bump_submodules \
	fetch_downloads tag_release

## Install clang lint dependencies
setup:
	sudo apt-get install -y clang-format-$(CLANG_VERSION) clang-tidy-$(CLANG_VERSION)

## Show this help menu
help:
	@awk 'BEGIN {FS = ":"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n\nTargets:\n"} \
	/^## / {desc = substr($$0, 4); next} \
	/^[a-zA-Z0-9_-]+:/ {if (desc) printf "  \033[36m%-22s\033[0m %s\n", $$1, desc; desc = ""; next} \
	{desc = ""}' $(MAKEFILE_LIST)

# BUILD
## Build for Linux using cmake
build_linux:
	cmake -B build \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DBUILD_MPQCLI=$(BUILD_MPQCLI)
	cmake --build build

## Build for Windows using cmake
build_windows:
	cmake -B build \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DBUILD_MPQCLI=$(BUILD_MPQCLI)
	cmake --build build --config $(CMAKE_BUILD_TYPE)

## Remove cmake build directory
build_clean:
	rm -rf build

## Generate compile_commands.json for clang-tidy
build_lint/compile_commands.json: CMakeLists.txt src/CMakeLists.txt
	cmake -B build_lint \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DBUILD_MPQCLI=ON \
		-DCMAKE_CXX_COMPILER=clang++-$(CLANG_VERSION) \
		-DCMAKE_CXX_FLAGS="--gcc-install-dir=$(GCC_INSTALL_DIR)"

## Remove cmake lint build directory
build_lint_clean:
	rm -rf build_lint

# DOCKER
## Build Docker image using musl
docker_musl_build:
	docker build -t mpqcli:$(VERSION) -f Dockerfile.musl .

## Run the musl Docker image
docker_musl_run:
	@docker run -it mpqcli:$(VERSION) version

## Build Docker image using glibc
docker_glibc_build:
	docker build -t mpqcli:$(VERSION) -f Dockerfile.glibc .

## Run the glibc Docker image
docker_glibc_run:
	@docker run -it mpqcli:$(VERSION) version

# TEST
## Create Python venv and install test dependencies
test_create_venv:
	python3 -m venv ./.venv
	. ./.venv/bin/activate && \
	pip3 install -r test/requirements.txt

## Run pytest test suite
test_mpqcli:
	. ./.venv/bin/activate && \
	python3 -m pytest test -s

## Remove test data directory
test_clean:
	rm -rf test/data

## Run ruff linter on test directory
test_lint:
	. ./.venv/bin/activate && \
	ruff check ./test

# LINT
## Check C++ formatting with clang-format
lint_format:
	find src \( -name "*.cpp" -o -name "*.h" \) \
	| xargs clang-format-$(CLANG_VERSION) --dry-run --Werror

## Auto-fix C++ formatting with clang-format
lint_format_fix:
	find src \( -name "*.cpp" -o -name "*.h" \) \
	| xargs clang-format-$(CLANG_VERSION) -i

## Run clang-tidy static analysis
lint_cpp: build_lint/compile_commands.json
	clang-tidy-$(CLANG_VERSION) \
	--quiet -p build_lint --header-filter="$(CURDIR)/src/.*" src/*.cpp

## Run all C++ linters
lint: lint_format lint_cpp

# CLEAN
## Remove all build and test artifacts
clean: build_clean build_lint_clean test_clean

# SUBMODULES
## Bump StormLib submodule to latest remote
bump_stormlib:
	@read -rp "[*] Bump StormLib? (y/N) " yn; \
	case $$yn in \
		[yY] ) git submodule update --init --remote extern/StormLib;; \
		* ) echo "[*] Skipping...";; \
	esac

## Bump CLI11 submodule to latest remote
bump_cli11:
	@read -rp "[*] Bump CLI11? (y/N) " yn; \
	case $$yn in \
		[yY] ) git submodule update --init --remote extern/CLI11;; \
		* ) echo "[*] Skipping...";; \
	esac

## Bump all submodules to latest remote
bump_submodules: bump_stormlib bump_cli11

# RELEASE
## Fetch package downloads and update README.md badge
fetch_downloads:
	@DOWNLOADS=$$(curl -s "$(PACKAGE_URL)" \
		| grep -A2 "Total downloads" \
		| grep -o '<h3 title="[0-9]*">[0-9]*</h3>' \
		| grep -o 'title="[0-9]*"' \
		| grep -o '[0-9]*' \
		| head -1); \
	sed -i "s/package_downloads-[0-9]*-green/package_downloads-$$DOWNLOADS-green/" $(README); \
	echo "[*] Updated package downloads badge: $$DOWNLOADS"
