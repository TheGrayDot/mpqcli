build_mpqcli:
	cd build; \
	cmake ..; \
	make

clean_mpqcli:
	rm -rf build
