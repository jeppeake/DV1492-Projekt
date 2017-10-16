#include "filesystem.h"

FileSystem::FileSystem() {
  createFolderi(0, "root");
  //createFile(1, "allocator"); proposition: allocator file will describe the file system, to make it easier to find unused space
  std::cout << "New filesystem created" << std::endl;

  directory_header dir(mMemblockDevice.readBlock(42));
}

FileSystem::~FileSystem() {
  std::cout << "Filesystem destroyed" << std::endl;
}

void FileSystem::createFile(){

}

void FileSystem::createFolderi(int parent, std::string name){

  //call allocation file to find empty space?

  int block = 0;

  std::vector<char> vec;

  directory_header dir(parent, block, name);

  dir.pack(vec);

  for(int i = vec.size(); i < 512; i++){//need to fill out rest of array with garbage, fix dynamic max allocation (512)
    vec.push_back(0);
  }

  std::cout << mMemblockDevice.writeBlock(42, vec) << std::endl;
}

void FileSystem::removeFile(){

}

void FileSystem::removeFolder(){

}

void FileSystem::goToFolder(){

}

void FileSystem::listDir(){

}
