mpq_extractor_build:
	g++ -o ./bin/mpq_extractor ./src/mpq_extractor.cpp -I/usr/local/include/ -lstorm -lz -lbz2

mpq_print_patchcmd_build:
	g++ -o ./bin/mpq_print_patchcmd ./src/mpq_print_patchcmd.cpp -I/usr/local/include/ -lstorm -lz -lbz2

patch_exe_to_mpq_build:
	g++ -o ./bin/patch_exe_to_mpq ./src/patch_exe_to_mpq.cpp
