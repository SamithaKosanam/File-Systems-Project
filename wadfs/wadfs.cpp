#include <fuse.h>
#include <cstring>
#include <errno.h>
#include "../libWad/Wad.cpp"

Wad *wad;

static int getattr_callback(const char* path, struct stat* stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    if (wad->isDirectory(path)) {
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
        return 0;
    }
    else if (wad->isContent(path)) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = wad->getSize(path);
        return 0;
    }

    return -ENOENT;
}

static int open_callback(const char* path, struct fuse_file_info* fi) {
    return 0;
}

static int read_callback(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	return wad->getContents(path, buf, size, offset);
}


/*static int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    if (!wad->isDirectory(path)) {
        return -ENOTDIR;
    }

    vector<string> *directory;
    int size = wad->getDirectory(path, directory);

    string element;
    for (int i=0; i<directory->size(); i++) {
	element = directory->at(i);
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, element.c_str(), NULL, 0);
    }


    return 0;
}*/

static int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    (void) offset;
    (void) fi;

    if (!wad->isDirectory(path)) {
        return -ENOTDIR;
    }

    vector<string> directory;
    int size = wad->getDirectory(path, &directory);

        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);

    string element;
    for (int i = 0; i < directory.size(); i++) {
        element = directory.at(i);
	filler(buf,element.c_str(), NULL, 0);
    }

    return 0;
}




static struct fuse_operations fuse_example_operations = {
        .getattr = getattr_callback,
        .open = open_callback,
        .read = read_callback,
        .readdir = readdir_callback,
};

int main(int argc, char* argv[]) {
	wad = Wad::loadWad(argv[argc-2]);
	if (wad == NULL) {
		return -1;
	}
	argv[argc-2] = argv[argc-1];
	argc--;
	int ret = fuse_main(argc, argv, &fuse_example_operations, NULL);
	delete wad;
	return ret;
}
