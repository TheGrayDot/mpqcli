build_mpqcli:
	cmake -B build; \
	cmake --build build

clean_mpqcli:
	rm -rf build
