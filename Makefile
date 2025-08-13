CMAKE_BUILD_TYPE := Release
BUILD_MPQCLI := ON
BUILD_PYTHON_WRAPPER := ON
VERSION := $(shell awk '/project\(MPQCLI VERSION/ {gsub(/\)/, "", $$3); print $$3}' CMakeLists.txt)

# BUILD
build_linux:
	cmake -B build \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DBUILD_MPQCLI=$(BUILD_MPQCLI) \
		-DBUILD_PYTHON_WRAPPER=$(BUILD_PYTHON_WRAPPER)
	cmake --build build

build_windows:
	cmake -B build \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DBUILD_MPQCLI=$(BUILD_MPQCLI) \
		-DBUILD_PYTHON_WRAPPER=$(BUILD_PYTHON_WRAPPER)
	cmake --build build --config $(CMAKE_BUILD_TYPE)

build_clean:
	rm -rf build

docker_build:
	docker build -t mpqcli:$(VERSION) .

docker_run:
	@docker run -it mpqcli:$(VERSION) version

# TEST
test_create_venv:
	python3 -m venv ./test/venv
	. ./test/venv/bin/activate && \
	pip3 install -r ./test/requirements.txt

test_mpqcli:
	. ./test/venv/bin/activate && \
	python3 -m pytest test -s

test_clean:
	rm -rf test/data

test_lint:
	. ./test/venv/bin/activate && \
	ruff check ./test
