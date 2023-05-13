wadfs: wadfs.cpp ../libWad/libWad.a
	g++ -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 wadfs.cpp -g -o wadfs -lfuse -L ../libWad -lWad -w
