//
// Created by samik on 4/16/2023.
//

#ifndef OS_P3_WAD_H
#define OS_P3_WAD_H

#include <cstring>
#include <string>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <cctype>
using namespace std;

struct Node {
    int offset;
    int length;
    string name;
    bool isDir;
    Node* parent = nullptr;

    Node(int offset, int length, string name, Node* parent, bool isDir=false) {
        this->offset = offset;
        this->length = length;
        this->name = name;
        this->parent = parent;
        this->isDir = isDir;
    }
};


class Wad {
private:
    // memory
    int memorySize;
    string magic;
    int numDescriptors;
    int descriptorOffset;
    unsigned char* memory;
    //map<string, Node> elements;

    Node* root = new Node(0, 0, "/", nullptr, true);
    vector<Node> elements;


public:

    //Object allocator; dynamically creates a Wad object and loads the WAD file data from path into memory.
//Caller must deallocate the memory using the delete keyword.
    static Wad* loadWad(const string &path);

    string getMagic();

//Returns true if path represents content (data), and false otherwise.
    bool isContent(const string &path);


//Returns true if path represents a directory, and false otherwise.
// For elements that have a length that is zero: These “marker” elements will be interpreted by the daemon as directories
    bool isDirectory(const string &path);


//If path represents content, returns the number of bytes in its data; otherwise, returns -1.
    int getSize(const string &path);


//If path represents content, copies as many bytes as are available, up to length, of content's data into the preexisting buffer. If offset is provided, data should be copied starting from that byte in the content. Returns
//number of bytes copied into buffer, or -1 if path does not represent content (e.g., if it represents a directory).
    int getContents(const string &path, char *buffer, int length, int offset = 0);


//If path represents a directory, places entries for immediately contained elements in directory. The elements
//should be placed in the directory in the same order as they are found in the WAD file. Returns the number of
//elements in the directory, or -1 if path does not represent a directory (e.g., if it represents content).
    int getDirectory(const string &path, vector<string> *directory);


};

#endif //OS_P3_WAD_H
