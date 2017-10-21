#include "filesystem.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <ostream>
#include <sstream>




FileSystem::FileSystem() {
  createFolderi(0, "root");
  /*createFolderi(0, "File1");
  createFolderi(1, "File2");

  //createFile(1, "allocator"); proposition: allocator file will describe the file system, to make it easier to find unused space
  std::cout << "New filesystem created" << std::endl;

  directory_header dir(mMemblockDevice.readBlock(0));

  directory_header dir2(mMemblockDevice.readBlock(2));
  //std::cout << getLocation(2) << std::endl;
  //std::cout << listDir(0) << std::endl;

  //std::cout << findByName(0,"File1") << std::endl;
  /*createFile(0, "Garbage");
  std::vector<char> vec;
  for(int i=0;i<100;i++){
    vec.push_back(i);
  }
  //std::cout << "vec: " << vec.size() << std::endl;
  appendData(0, "Garbage", vec);*/

  //std::cout << "First empty: " << findEmptyBlock() << std::endl;

  createFile(0, "Lorem");
  createFile(0, "Lorem2");

  std::stringstream ss;

  for(int i=0; i < 300; i++){
    ss << "A" << i << " ";
  }
  std::string test = ss.str();

  std::vector<char> to_append(test.begin(), test.end());
  to_append.push_back('\0');
  appendData(0, "Lorem", to_append);
  ss.str("");

  for(int i=0; i < 300; i++){
    ss << "B" << i << " ";
  }
  test = ss.str();
  std::vector<char> to_append2(test.begin(), test.end());
  appendData(0, "Lorem2", to_append2);
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

int FileSystem::createFile(int parent, std::string name){
  //call allocation file to find empty space
  std::vector<char> vec;
  int block = findEmptyBlock();
  file_header dir(parent, block, block_size, name);

  dir.pack(vec);
  //std::cout << vec.size() << std::endl;
  for(int i = vec.size(); i < block_size; i++){
    vec.push_back(0);
  }

  mMemblockDevice.writeBlock(block, vec);

  if(parent != -1){
    directory_header parentBlock(mMemblockDevice.readBlock(parent));//need to load parent to write new data
    parentBlock.addChild(block);

    std::vector<char> vec2;
    parentBlock.pack(vec2);
    for(int i = vec2.size(); i < block_size; i++){
      vec2.push_back(0);
    }
    mMemblockDevice.writeBlock(parent, vec2);
  }

  return block;
}

int FileSystem::createFolderi(int parent, std::string name){

  //call allocation file to find empty space

  //int block = 0;

  std::vector<char> vec;
  int block = findEmptyBlock();
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

  return block;
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
  while(currDir != NULL){
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
              return -2;//not directory error
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
  delete head;
  return 1;
}

int FileSystem::appendData(int loc, std::string name, std::vector<char> content){
  loc = findByName(loc, name);
  if(loc < 0){
    return loc;
  }
  Block block;
  block = mMemblockDevice.readBlock(loc);
  if(block[0] != FILE){
    return -2;
  }
  file_header* file = new file_header(block);
  //std::cout << file->CB << std::endl;
  while(file->CB != -1){
    //std::cout << "Current: " << loc << std::endl;
    //std::cout << "Next: " << file->CB << std::endl;
    //find bottom
    loc = file->CB;
    block = mMemblockDevice.readBlock(loc);
    delete file;
    file = new file_header(block);
  }
  delete file;

  //std::cout << "Current: " << loc << std::endl;
  //std::cout << "Next: " << file->CB << std::endl;

  int file_overflows = 0;
  int overflow = 0;
  while(overflow != content.size()){

    Block primaryBlock = mMemblockDevice.readBlock(loc);

    //std::cout << "Overflows: " << file_overflows << std::endl;
    //std::cout << "Overflow (loop): " << overflow << std::endl;
    //std::cout << "Current block: " << loc << std::endl;
    //std::cout << "Data left to write (pre write): " << content.size() - overflow << std::endl;
    //std::cout << "Current: " << loc << std::endl;
    //std::cout << "Next: " << file->CB << std::endl;
    file_header currentFile(primaryBlock);
    //int newLoc = createFile(-1, parentFile.name);
    std::vector<char> vec;
    int original_size = currentFile.pack(vec);

    for(int i=overflow;i<content.size();i++){
      currentFile.content.push_back(content.at(i));
      //std::cout << content.at(i);
    }
    //std::cout << std::endl;
    vec.clear();
    int new_size = currentFile.pack(vec);
    overflow += new_size - original_size;
    int newLoc = -1;
    //std::cout << "Data left to write (post write): " << content.size() - overflow << std::endl;
    if(overflow != content.size()){
      //new file
      //std::cout << "Overflowing" << std::endl;
      newLoc = createFile(-1, currentFile.name);

      currentFile.CB = newLoc;
      //std::cout << "Created new sub-file at: " << newLoc << std::endl;
      vec.clear();
      currentFile.pack(vec);

      file_overflows++;
    }
    for(int i=vec.size();i<block_size;i++){
      vec.push_back(0);
    }
    int result = mMemblockDevice.writeBlock(loc, vec);
    //std::cout << result << std::endl;
    loc = newLoc;
  }
  return loc;
}

int FileSystem::findEmptyBlock(){
  for(int i = 0; i < mMemblockDevice.size(); i++){
    if(mMemblockDevice.readBlock(i)[0] == 0){
      return i;
    }
  }
  return -1;
}

std::string FileSystem::readFile(int loc, std::string name){
  loc = findByName(loc, name);
  Block block = mMemblockDevice.readBlock(loc);
  std::stringstream ss;
  if(block[0] != FILE){
    ss << name << " is not a file." << std::endl;
    return ss.str();
  }
  file_header* file = new file_header(block);
  int next = file->CB;
  bool running = true;
  do{
    //std::cout << "next: " << next << std::endl;
    //std::cout << "Content size (actual): " << file->content.size() << std::endl;
    //std::cout << "Content size (coded): " << file->content_length << std::endl;
    for(unsigned int i = 0; i < file->content.size(); i++){
      ss << file->content.at(i);
    }
    if(next != -1){
      block = mMemblockDevice.readBlock(next);
      file = new file_header(block);
      next = file->CB;
    }else{
      running = false;
    }
  }while(running);
  delete file;
  return ss.str();
}

void FileSystem::saveToFile(std::string path)
{
  std::ofstream out;
  out.open(path);
  for(int i=0;i<mMemblockDevice.size();i++)
  {
    for(int j=0;j<block_size;j++)
      out << mMemblockDevice.readBlock(i).toArray()[j];
  }
}

int FileSystem::loadFromFile(std::string path)
{
  std::ifstream t(path);
  int r = 0;
  if(!t.good())
  {
    r = -1;
    return;
  }
  std::string raw = std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  std::stringstream stream(raw);
  mMemblockDevice.reset();
  for(int i=0;i<mMemblockDevice.size();i++)
  {
    std::string blockstr;
    for(int j=0;j<block_size;j++)
    {
      blockstr += raw[j+(i*block_size)];
    }
    mMemblockDevice.writeBlock(i,blockstr);
  }
  return r;
}
