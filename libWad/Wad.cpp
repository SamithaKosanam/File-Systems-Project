#include "Wad.h"

//Object allocator; dynamically creates a Wad object and loads the WAD file data from path into memory.
//Caller must deallocate the memory using the delete keyword.
Wad* Wad::loadWad(const string &path) {
    // Open the file for binary reading
    ifstream file(path, ios::binary);
    // Get the size of the file
    file.seekg(0, ios::end);
    // change int to size_t if not working
    int memorySize = file.tellg();
    file.seekg(0, ios::beg);
    // Allocate memory to hold the file data
    char* memory = new char[memorySize];
    // Read the entire file into memory
    file.read(memory, memorySize);
    // Close the file
    file.close();
    // Create a new Wad object using the data


    Wad* wad = new Wad();
    wad->memory = (unsigned char*)memory;
    wad->memorySize = memorySize;

    for (int i=0; i<4; i++)
        wad->magic += memory[i];
    

    int num;

    /*for (int i=0; i<4; i++)
        num += (int)wad->memory[4+i];
	*/
    memcpy((char*)&wad->numDescriptors, (char*)&wad->memory[4], 4);
    //wad->numDescriptors = num;

    int offset;
/*    for (int i=0; i<4; i++)
        offset += (int)wad->memory[8+i];
  */  //wad->descriptorOffset = offset;
    memcpy((char*)&wad->descriptorOffset, (char*)&wad->memory[8], 4);

    // create tree
    int off;
    int length;
    string name;
    Node* create;
    Node* current_directory = wad->root;
    wad->elements.push_back(*(wad->root));
    int start = 0;
    bool count = false;
    for (int i = wad->descriptorOffset; i < (wad->numDescriptors*16)+wad->descriptorOffset; i+=16) {
        off = 0;
        length = 0;
        name = "";
	memcpy((char*)&off, (char*)&wad->memory[i], 4);
	memcpy((char*)&length, (char*)&wad->memory[i+4], 4);
/*        for (int j=0; j<4; j++)
            off += (int)wad->memory[i+j];
        for (int j=0; j<4; j++)
            length += (int)wad->memory[i+j+4];
*/
        for (int j=0; j<8; j++)
            name += wad->memory[i+j+8];

        if (length == 0) {
            if (name.find("_START") != string::npos) {
                create = new Node(off, length, name.substr(0, name.find("_START")), current_directory, true);
                //elements[name.substr(0, name.length() - 5)] = *create;
                wad->elements.push_back(*create);
                current_directory = create;
            }
            else if (isalpha(name.at(0)) !=0 && isalpha(name.at(2)) !=0 && isdigit(name.at(1)) !=0 && isdigit(name.at(3)) !=0) {
                create = new Node(off, length, name.substr(0, 4), current_directory, true);
                //elements[name] = *create;
                wad->elements.push_back(*create);
                current_directory = create;
                count = true;
            }
            else if (name.find("_END") != string::npos){
                current_directory = current_directory->parent;
            }
        }
        else {
            if (count) {
                start++;
                create = new Node(off, length, name.substr(0, name.find('\000')), current_directory);
                wad->elements.push_back(*create);
                if (start == 10) {
                    count = false;
                    start = 0;
                    current_directory = current_directory->parent;
                }
            }
            else {
                create = new Node(off, length, name.substr(0, name.find('\000')), current_directory);
                wad->elements.push_back(*create);
            }
        }
    }

    return wad;
}

string Wad::getMagic() {
    return magic;
}


//Returns true if path represents content (data), and false otherwise.
bool Wad::isContent(const string &path) {
    string branch;
    if (path == "/")
        return true;
    else {
        for (int i = 0; i < path.length(); i++) {
            if (path.at(i) != '/')
                branch += path.at(i);
            else
                branch = "";
        }
    }
    for (int i=0; i<elements.size(); i++) {
        if (elements.at(i).name == branch && !elements.at(i).isDir) {
            return true;
        }
    }
    return false;
}


//Returns true if path represents a directory, and false otherwise.
// For elements that have a length that is zero: These “marker” elements will be interpreted by the daemon as directories
bool Wad::isDirectory(const string &path) {
    string branch;
    if (path == "/")
        return true;
    else {
        for (int i = 0; i < path.length(); i++) {
            if (path.at(i) != '/')
                branch += path.at(i);
            else
                branch = "";
        }
    }
    for (int i=0; i<elements.size(); i++) {
        if (elements.at(i).name == branch && elements.at(i).isDir) {
            return true;
        }
    }
    return false;
}


//If path represents content, returns the number of bytes in its data; otherwise, returns -1.
int Wad::getSize(const string &path) {
    string branch;
    if (path == "/")
        branch = path;
    else {
        for (int i = 0; i < path.length(); i++) {
            if (path.at(i) != '/')
                branch += path.at(i);
            else
                branch = "";
        }
    }

    if (isContent(path)) {
        for (int i=0; i<elements.size(); i++) {
            if (elements.at(i).name == branch)
                return elements.at(i).length;
        }
    }
    return -1;
}


//If path represents content, copies as many bytes as are available, up to length, of content's data into the preexisting buffer. If offset is provided, data should be copied starting from that byte in the content. Returns
//number of bytes copied into buffer, or -1 if path does not represent content (e.g., if it represents a directory).
int Wad::getContents(const string &path, char *buffer, int length, int offset) {
    if (!isContent(path))
        return -1;

    string branch;
    if (path == "/")
        branch = path;
    else {
        for (int i = 0; i < path.length(); i++) {
            if (path.at(i) != '/')
                branch += path.at(i);
            else
                branch = "";
        }
    }
    int currLength = 0;
    int currOff = 0;
    for (int i=0; i<elements.size(); i++) {
        if (elements.at(i).name == branch) {
            currLength = elements.at(i).length;
            currOff = elements.at(i).offset;
        }
    }
    buffer = new char[length];
    int index = 0;
    // + descriptor offset for starting i ??
    for (int i=0; i<length && i<currLength; i++) {
        buffer[index++] = memory[currOff + offset + i];
    }
    return index;
}


//If path represents a directory, places entries for immediately contained elements in directory. The elements
//should be placed in the directory in the same order as they are found in the WAD file. Returns the number of
//elements in the directory, or -1 if path does not represent a directory (e.g., if it represents content).
int Wad::getDirectory(const string &path, vector<string> *directory) {
    int ret = 0;
    if (path!="/" && !isDirectory(path.substr(0, path.length()-1)))
        return -1;
    else {
        string branch;
        if (path == "/")
            branch = path;
        else {
            for (int i = 0; i < path.length()-1; i++) {
                if (path.at(i) != '/')
                    branch += path.at(i);
                else
                    branch = "";
            }
        }
        for (int i = 1; i < elements.size(); i++) {
            if (elements.at(i).parent!=nullptr && elements.at(i).parent->name == branch) {
                directory->push_back(elements.at(i).name);
                ret++;
            }
        }

    }
    return ret;
}
