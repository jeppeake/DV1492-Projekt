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

  //std::cout << findByName(0,"File1") << std::endl;

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

void FileSystem::createFile(int parent, std::string name){
  //call allocation file to find empty space
  std::vector<char> vec;

  file_header dir(parent, block, name);

  dir.pack(vec);
  //std::cout << vec.size() << std::endl;
  for(int i = vec.size(); i < block_size; i++){
    vec.push_back(0);
  }

  mMemblockDevice.writeBlock(block, vec);


  directory_header parentBlock(mMemblockDevice.readBlock(parent));//need to load parent to write new data
  parentBlock.addChild(block);

  std::vector<char> vec2;
  parentBlock.pack(vec2);
  for(int i = vec2.size(); i < block_size; i++){
    vec2.push_back(0);
  }
  mMemblockDevice.writeBlock(parent, vec2);

  block++;
}

void FileSystem::createFolderi(int parent, std::string name){

  //call allocation file to find empty space

  //int block = 0;

  std::vector<char> vec;

  directory_header dir(parent, block, name);

  dir.pack(vec);
  //std::cout << vec.size() << std::endl;
  for(int i = vec.size(); i < block_size; i++){
    vec.push_back(0);
  }

  mMemblockDevice.writeBlock(block, vec);


  directory_header parentBlock(mMemblockDevice.readBlock(parent));//need to load parent to write new data
  parentBlock.addChild(block);

  std::vector<char> vec2;
  parentBlock.pack(vec2);
  //std::cout << vec2.size() << std::endl;
  for(int i = vec2.size(); i < block_size; i++){
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

  if(strcmp(currDir,"root") == 0){
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
    }else if(strcmp(currDir,".") == 0){//move up one
      loc = USH.parent;
    }else{//search for child (move down)
        directory_header dir(block);
        bool found = false;
        for(unsigned int i = 0; i < dir.children.size(); i++){
          block = mMemblockDevice.readBlock(dir.children.at(i));
          unspecified_header USH2(block);
          if(USH2.name == currDir){
            loc = dir.children.at(i);
            found = true;
            block = mMemblockDevice.readBlock(loc);
            unspecified_header USH3(block);
            if(USH3.type != DIRECTORY){
              return -2;
            }
          }
        }
        if(!found){
          return -1;//not found header
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
  s = getLocation(loc);
  for(unsigned int i=0;i<dir.children.size();i++){
    block = mMemblockDevice.readBlock(dir.children.at(i));
    unspecified_header head(block);
    s += "\n  " + head.name;
  }

  return s;
}

int FileSystem::findByName(int loc, std::string name){
  Block block = mMemblockDevice.readBlock(loc);
  if(block[0] != 1){
    return -2;
  }
  directory_header dir(block);
  for(unsigned int i = 0; i < dir.children.size(); i++){
    block = mMemblockDevice.readBlock(dir.children.at(i));
    unspecified_header USH(block);
    if(USH.name == name){
      return dir.children.at(i);
    }
  }
  return -1;
}

int FileSystem::editHeader(int loc, std::string name){
  Block block = mMemblockDevice.readBlock(loc);
  header* head;
  if(block[0] == 1){
    head = new directory_header(block);
  }else if(block[0] == 2){
    head = new file_header(block);
  }else{
    return -2;
  }
  head->name = name;

  std::vector<char> vec;

  head->pack(vec);

  for(int i = vec.size(); i < block_size; i++){
    vec.push_back(0);
  }

  mMemblockDevice.writeBlock(loc, vec);
}
