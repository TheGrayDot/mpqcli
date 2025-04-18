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

test_mpqcli:
	cd test; \
	. ./venv/bin/activate && \
	python3 -m pytest -s

test_clean:
	rm -rf test/data
