# MPQ_FILE = $(shell pwd)/test/WoW-1.6.1.4544-to-1.7.0.4671-enUS-patch.exe
# MPQ_FILE = $(shell pwd)/test/WoW-1.12.0.5595-to-1.12.1.5875-enUS-patch.exe
# MPQ_FILE = $(shell pwd)/test/WoW-2.0.4.6314-to-2.0.5.6320-enUS-patch.exe
MPQ_FILE = $(shell pwd)/test/WoW-3.0.3.9183-to-3.0.8.9464-enUS-patch.exe

build_mpqcli:
	mkdir -p build; \
	cd build; \
	cmake ..; \
	make

run_mpqcli_info:
	./build/bin/mpqcli info $(MPQ_FILE)

run_mpqcli_verify:
	./build/bin/mpqcli verify $(MPQ_FILE)

run_mpqcli_patch:
	./build/bin/mpqcli patch $(MPQ_FILE)

run_mpqcli_patch_extract:
	./build/bin/mpqcli patch -e $(MPQ_FILE)
