CMAKE_BUILD_TYPE := Release
BUILD_MPQCLI := ON
VERSION := $(shell awk '/project\(MPQCLI VERSION/ {gsub(/\)/, "", $$3); print $$3}' CMakeLists.txt)
README := README.md
PACKAGE_URL := https://github.com/TheGrayDot/mpqcli/pkgs/container/mpqcli

.PHONY: help \
	build_linux build_windows build_clean \
	docker_musl_build docker_musl_run docker_glibc_build docker_glibc_run \
	test_create_venv test_mpqcli test_clean test_lint \
	clean \
	bump_stormlib bump_cli11 bump_submodules \
	fetch_downloads tag_release

help: ## Show this help menu
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n\nTargets:\n"} \
	/^[a-zA-Z0-9_-]+:.*?##/ { printf "  \033[36m%-22s\033[0m %s\n", $$1, $$2 }' $(MAKEFILE_LIST)

# BUILD
build_linux: ## Build for Linux using cmake
	cmake -B build \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DBUILD_MPQCLI=$(BUILD_MPQCLI)
	cmake --build build

build_windows: ## Build for Windows using cmake
	cmake -B build \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DBUILD_MPQCLI=$(BUILD_MPQCLI)
	cmake --build build --config $(CMAKE_BUILD_TYPE)

build_clean: ## Remove cmake build directory
	rm -rf build

# DOCKER
docker_musl_build: ## Build Docker image using musl
	docker build -t mpqcli:$(VERSION) -f Dockerfile.musl .

docker_musl_run: ## Run the musl Docker image
	@docker run -it mpqcli:$(VERSION) version

docker_glibc_build: ## Build Docker image using glibc
	docker build -t mpqcli:$(VERSION) -f Dockerfile.glibc .

docker_glibc_run: ## Run the glibc Docker image
	@docker run -it mpqcli:$(VERSION) version

# TEST
test_create_venv: ## Create Python venv and install test dependencies
	python3 -m venv ./.venv
	. ./.venv/bin/activate && \
	pip3 install -r test/requirements.txt

test_mpqcli: ## Run pytest test suite
	. ./.venv/bin/activate && \
	python3 -m pytest test -s

test_clean: ## Remove test data directory
	rm -rf test/data

test_lint: ## Run ruff linter on test directory
	. ./.venv/bin/activate && \
	ruff check ./test

# CLEAN
clean: build_clean test_clean ## Remove all build and test artifacts

# SUBMODULES
bump_stormlib: ## Bump StormLib submodule to latest remote
	@read -rp "[*] Bump StormLib? (y/N) " yn; \
	case $$yn in \
		[yY] ) git submodule update --init --remote extern/StormLib;; \
		* ) echo "[*] Skipping...";; \
	esac

bump_cli11: ## Bump CLI11 submodule to latest remote
	@read -rp "[*] Bump CLI11? (y/N) " yn; \
	case $$yn in \
		[yY] ) git submodule update --init --remote extern/CLI11;; \
		* ) echo "[*] Skipping...";; \
	esac

bump_submodules: bump_stormlib bump_cli11 ## Bump all submodules to latest remote

# RELEASE
fetch_downloads: ## Fetch package downloads and update README.md badge
	@DOWNLOADS=$$(curl -s "$(PACKAGE_URL)" \
		| grep -A2 "Total downloads" \
		| grep -o '<h3 title="[0-9]*">[0-9]*</h3>' \
		| grep -o 'title="[0-9]*"' \
		| grep -o '[0-9]*' \
		| head -1); \
	sed -i "s/package_downloads-[0-9]*-green/package_downloads-$$DOWNLOADS-green/" $(README); \
	echo "[*] Updated package downloads badge: $$DOWNLOADS"

tag_release: ## Tag and push the current project version
	@echo "[*] Current version: v$(VERSION)"
	@read -rp "[*] Tag and Release? (y/N) " yn; \
	case $$yn in \
		[yY] ) git tag "v$(VERSION)" && git push --tags && echo "[*] Tagged and pushed v$(VERSION)";; \
		[nN] ) echo "[*] Exiting...";; \
		* ) echo "[*] Invalid response... Exiting"; exit 1;; \
	esac
