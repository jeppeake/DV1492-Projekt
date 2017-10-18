#include "filesystem.h"

FileSystem::FileSystem() {
  createFolderi(0, "root");
  createFolderi(0, "File1");
  createFolderi(1, "File2");

  //createFile(1, "allocator"); proposition: allocator file will describe the file system, to make it easier to find unused space
  std::cout << "New filesystem created" << std::endl;

  directory_header dir(mMemblockDevice.readBlock(0));
  /*for(unsigned int i = 0;i < dir.children.size();i++){
    std::cout << "Child: " << i << " : " << (int)dir.children.at(i) << std::endl;
  }*/
  directory_header dir2(mMemblockDevice.readBlock(2));
  //std::cout << getLocation(2) << std::endl;
  //std::cout << listDir(0) << std::endl;
}

FileSystem::~FileSystem() {
  std::cout << "Filesystem destroyed" << std::endl;
}

std::string FileSystem::getLocation(int loc){
  std::string s;
  bool looping = true;
  while(looping){//if parent == block then we have reached root
    Block block = mMemblockDevice.readBlock(loc);
    std::vector<char> data;
    for(int i=0; i < block.size(); i++){
      data.push_back(block[i]);
    }
    unspecified_header head(block);
    loc = head.parent;
    if(head.parent == head.block){
      looping = false;
      s = head.name + s;
    }else{
      s = "/" + head.name + s;
    }
  }

  return s;
}

void FileSystem::createFile(){

}

void FileSystem::createFolderi(int parent, std::string name){

  //call allocation file to find empty space?

  //int block = 0;

  std::vector<char> vec;

  directory_header dir(parent, block, name);

  dir.pack(vec);

  for(int i = vec.size(); i < 512; i++){//need to fill out rest of array with garbage, fix dynamic max allocation (512)
    vec.push_back(0);
  }

  mMemblockDevice.writeBlock(block, vec);


  directory_header parentBlock(mMemblockDevice.readBlock(parent));//need to load parent to write new data
  parentBlock.addChild(block);

  std::vector<char> vec2;
  parentBlock.pack(vec2);
  //std::cout << vec2.size() << std::endl;
  for(int i = vec2.size(); i < 512; i++){//need to fill out rest of array with garbage, fix dynamic max allocation (512)
    vec2.push_back(0);
  }
  mMemblockDevice.writeBlock(parent, vec2);

  block++;
}

void FileSystem::removeFile(){

}

void FileSystem::removeFolder(){

}

int FileSystem::goToFolder(std::string path, int loc){
  Block block;

  char* currDir;
  currDir = strtok(&path[0],"/");
  bool absolute_path = false;
  if(strcmp(currDir,"root") == 0){
    absolute_path = true;
  }
  if(absolute_path){
    loc = 0;
  }
  //std::cout << "AP: " << absolute_path << std::endl;
  while(currDir != NULL){
    //std::cout << currDir << std::endl;
    Block block;
    block = mMemblockDevice.readBlock(loc);
    unspecified_header USH(block);
    if(strcmp(currDir,"..") == 0){//move up two
      block = mMemblockDevice.readBlock(USH.parent);
      unspecified_header USH2(block);
      loc = USH2.parent;
      //std::cout << "Operating with .." << std::endl;
    }else if(strcmp(currDir,".") == 0){//move up one
      loc = USH.parent;
      //std::cout << "Operating with ." << std::endl;
    }else{//search for child (move down)
      if(USH.type == 1){
        directory_header dir(block);
        bool found = false;
        for(unsigned int i = 0; i < dir.children.size(); i++){
          block = mMemblockDevice.readBlock(dir.children.at(i));
          unspecified_header USH2(block);
          if(USH2.name == currDir){
            loc = dir.children.at(i);
            found = true;
          }
        }
        if(!found){
          return -1;//not found header
        }
      }else{
        return -2;//non-directory error
      }
    }
    currDir = strtok(NULL,"/");
  }
  return loc;
}

std::string FileSystem::listDir(int loc){
  std::string s;

  Block block = mMemblockDevice.readBlock(loc);
  std::vector<char> data;
  for(int i=0; i < block.size(); i++){
    data.push_back(block[i]);
  }

  if(data.at(0) != 1){
    s = "This is not a directory!";
  }

  directory_header dir(block);
  s = getLocation(loc) + "\n";
  for(unsigned int i=0;i<dir.children.size();i++){
    block = mMemblockDevice.readBlock(dir.children.at(i));
    unspecified_header head(block);
    s += "  " + head.name + "\n";
  }

  return s;
}
