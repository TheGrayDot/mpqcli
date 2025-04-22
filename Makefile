VERSION := $(shell awk '/project\(MPQCLI VERSION/ {gsub(/\)/, "", $$3); print $$3}' CMakeLists.txt)

build_linux_debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -B build; \
	cmake --build build

build_linux_release:
	cmake -B build; \
	cmake --build build

build_windows_debug:
	cmake -B build; \
	cmake --build build --config Debug

build_windows_release:
	cmake -B build; \
	cmake --build build --config Release

build_clean:
	rm -rf build

docker_build:
	docker build -t mpqcli:$(VERSION) .

docker_run:
	@docker run -it mpqcli:$(VERSION) about

test_mpqcli:
	. ./test/venv/bin/activate && \
	python3 -m pytest test -s

test_clean:
	rm -rf test/data
