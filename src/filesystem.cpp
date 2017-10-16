#include "filesystem.h"

FileSystem::FileSystem() {
  createFolderi(0, 0, 0, "root");
  std::cout << "New filesystem created" << std::endl;
}

FileSystem::~FileSystem() {
  std::cout << "Filesystem destroyed" << std::endl;
}

void FileSystem::createFile(){

}

void FileSystem::createFolderi(int parent, int id, int block, std::string name){
  std::vector<char> vec;
  
  directory_header dir(parent,id,block,name);

  mMemblockDevice.writeBlock(block, vec);
}

void FileSystem::removeFile(){

}

void FileSystem::removeFolder(){

}

void FileSystem::goToFolder(){

}

void FileSystem::listDir(){

}
