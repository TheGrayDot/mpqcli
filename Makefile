build_mpqcli_linux:
	cmake -B build; \
	cmake --build build

build_mpqcli_windows:
	cmake -B build; \
	cmake --build build --config Release

clean_mpqcli:
	rm -rf build
