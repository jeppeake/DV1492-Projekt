#include "filesystem.h"

#include <iostream>
#include <fstream>
#include <streambuf>
#include <ostream>
#include <sstream>




FileSystem::FileSystem() {
  createFolderi(0, "root");
}

FileSystem::~FileSystem() {

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
  std::vector<char> vec;
  int block = findEmptyBlock();
  file_header dir(parent, block, block_size, name);

  dir.pack(vec);
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

int FileSystem::removeFile(int curLoc, std::string name)
{
  Block currBlock =  mMemblockDevice.readBlock(curLoc);
  directory_header curr(currBlock);
  int loc = findByName(curLoc,name);
  if(!loc)
    return -1;
  Block block = mMemblockDevice.readBlock(loc);
  unspecified_header head(block);
  if(head.type == DIRECTORY)
  {
    return removeFolder(loc, curLoc);
  }
  std::vector<char> emptyVec;
  for(int c=0;c<block_size;c++)
    emptyVec.push_back(0);

  for(int i=0;(unsigned int)i<curr.children.size();i++)
  {
    Block child = mMemblockDevice.readBlock(curr.children.at(i));
    file_header currFile(child);
    if(currFile.name == name && currFile.type == FILE)
    {
      int childLoc = curr.children.at(i);
      curr.children.erase(curr.children.begin() + i); //remove references to child in parent directory header
      while(childLoc != -1) //if files are larger than 1 block, step through their continued blocks until we hit the last 1.
      {
        mMemblockDevice.writeBlock(childLoc,emptyVec); //blank file block
        if(currFile.CB != -1)
        {
          childLoc = currFile.CB;
          child = mMemblockDevice.readBlock(currFile.CB);
          currFile = file_header(child);
        }
        else
          childLoc = -1;
      }
    }
  }
  std::vector<char> vec;
  vec.clear();
  curr.pack(vec);
  for(int i = vec.size(); i < block_size; i++){
    vec.push_back(0);
  }
  mMemblockDevice.writeBlock(curLoc,vec);
  return 1;
}

int FileSystem::removeFolder(int loc, int parent)
{
  Block block = mMemblockDevice.readBlock(loc);
  directory_header dir(block);

  for(int i = 0; (unsigned int)i < dir.children.size(); i++)
  {
    Block child = mMemblockDevice.readBlock(dir.children.at(i));
    unspecified_header ch(child);

    std::string thename = ch.name;
    removeFile(loc,thename); //remove all files in this directory, if it includes other directories we will end up here again recursively.
  }

  Block par = mMemblockDevice.readBlock(parent);
  directory_header pd(par);
  std::vector<char> vec;
  for(int j = 0; (unsigned int)j < pd.children.size(); j++) //remove from parent's child list first
  {
    Block pc = mMemblockDevice.readBlock(pd.children.at(j));
    unspecified_header pch(pc);
    if(pch.name == dir.name)
    {
      pd.children.erase(pd.children.begin() + j);
    }
  }

  vec.clear();
  pd.pack(vec);
  for(int k = vec.size(); k < block_size; k++)
    vec.push_back(0);
  mMemblockDevice.writeBlock(parent,vec);

  std::vector<char> emptyVec;
  for(int c=0;c<block_size;c++)
    emptyVec.push_back(0);

  mMemblockDevice.writeBlock(loc,emptyVec);

  return 1;
}

int FileSystem::goToFolder(std::string path, int loc){
  Block block;
  char* currDir;
  currDir = strtok(&path[0],"/");
  if(currDir == NULL){
    return -1;//empty path error
  }
  if(strcmp(currDir,"root") == 0){
    loc = 0;
  }
  while(currDir != NULL){
    Block block;
    block = mMemblockDevice.readBlock(loc);
    unspecified_header USH(block);
    if(strcmp(currDir,"..") == 0){//move up two
      loc = USH.parent;
    }else if(strcmp(currDir,".") == 0){//move up one
      loc = loc;
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
          return -1;//not found error
        }
    }
    currDir = strtok(NULL,"/");
  }
  delete currDir;
  return loc;
}

std::string FileSystem::listDir(int loc){
  std::stringstream ss;
  Block block = mMemblockDevice.readBlock(loc);
  std::vector<char> data;
  for(int i=0; i < block.size(); i++){
    data.push_back(block[i]);
  }
  if(data.at(0) != 1){
    ss << "This is not a directory!";
    return ss.str();
  }
  directory_header dir(block);
  ss << getLocation(loc);
  for(unsigned int i=0;i<dir.children.size();i++){
    block = mMemblockDevice.readBlock(dir.children.at(i));
    unspecified_header head(block);
    ss << std::endl << "    " << head.name << "  :  " << (getSize(dir.block, head.name) * block_size) << " Byte.";
  }
  return ss.str();
}

int FileSystem::findByName(int loc, std::string name){
  Block block = mMemblockDevice.readBlock(loc);
  if(block[0] != 1){
    //return -2;
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
  while(file->CB != -1){
    //find bottom
    loc = file->CB;
    block = mMemblockDevice.readBlock(loc);
    delete file;
    file = new file_header(block);
  }
  delete file;

  int file_overflows = 0;
  int overflow = 0;
  while((unsigned int)overflow != content.size()){
    Block primaryBlock = mMemblockDevice.readBlock(loc);
    file_header currentFile(primaryBlock);
    std::vector<char> vec;
    int original_size = currentFile.pack(vec);

    for(int i=overflow;(unsigned int)i<content.size();i++){
      currentFile.content.push_back(content.at(i));
    }

    vec.clear();
    int new_size = currentFile.pack(vec);
    overflow += new_size - original_size;
    int newLoc = -1;

    if((unsigned int)overflow != content.size()){
      //new file
      newLoc = createFile(-1, currentFile.name);
      currentFile.CB = newLoc;
      vec.clear();
      currentFile.pack(vec);
      file_overflows++;
    }
    for(int i=vec.size();i<block_size;i++){
      vec.push_back(0);
    }
    mMemblockDevice.writeBlock(loc, vec);
    loc = newLoc;
  }
  return 1;
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
  if(loc == -1){
      return "File not found.";
  }
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
  if(!t.good())
  {
    return 0;
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
  return 1;
}

int FileSystem::altremoveFile(int loc, std::string name){
  loc = findByName(loc, name);
  if(loc < 0){
    return loc;
  }
  Block block = mMemblockDevice.readBlock(loc);
  if(block[0] != FILE){
    return -2;
  }

  unspecified_header head(block);
  block = mMemblockDevice.readBlock(head.parent);
  directory_header dir(block);
  dir.removeChild(loc);
  std::vector<char> remChild;
  dir.pack(remChild);
  for(int i = remChild.size(); i < block_size; i++){
    remChild.push_back(0);
  }
  mMemblockDevice.writeBlock(head.parent, remChild);

  do{
    block = mMemblockDevice.readBlock(loc);
    file_header file(block);
    int newLoc = file.CB;
    std::vector<char> vec;
    for(int i=0;i<block_size;i++){
      vec.push_back(0);
    }
    mMemblockDevice.writeBlock(loc, vec);
    loc = newLoc;
  }while(loc != -1);
  return loc;
}

int FileSystem::copy(int fromLoc, std::string from, std::string to){
  int checkLoc = findByName(fromLoc, from);
  if(checkLoc < 0){
    return checkLoc;
  }
  int toLoc = goToFolder(to, fromLoc);
  if(toLoc < 0){
    return toLoc - 1;
  }

  if(checkLoc == toLoc){
    return -4;
  }

  //std::cout << toLoc << " " << fromLoc << std::endl;
  Block block = mMemblockDevice.readBlock(checkLoc);
  if(block[0] == 1){
    //std::cout << "dir" << std::endl;
    copyDirectory(toLoc,fromLoc, from);
  }
  if(block[0] == 2){
    //std::cout << "file" << std::endl;
    copyFile(toLoc,fromLoc,from);
  }
  return toLoc;
}

int FileSystem::copyDirectory(int parent, int loc, std::string name){
  int newLoc = createFolderi(parent, name);
  loc = findByName(loc, name);
  Block block = mMemblockDevice.readBlock(loc);
  directory_header dir(block);
  for(int i = 0;(unsigned int)i < dir.children.size();i++){
    int childLoc = dir.children.at(i);
    block = mMemblockDevice.readBlock(childLoc);
    unspecified_header head(block);
    if(head.type == 1){
      copyDirectory(newLoc, loc, head.name);
    }
    if(head.type == 2){
      copyFile(newLoc, loc, head.name);
    }
  }
  return newLoc;
}

int FileSystem::copyFile(int parent, int loc, std::string name){
  std::string data = readFile(loc, name);
  loc = createFile(parent, name);

  std::vector<char> to_append(data.begin(), data.end());
  to_append.push_back('\0');

  appendData(parent, name, to_append);

  return loc;
}

int FileSystem::getSize(int loc, std::string name){
  int checkLoc = findByName(loc, name);
  if(checkLoc < 0){
    return checkLoc;
  }
  Block block = mMemblockDevice.readBlock(checkLoc);
  unspecified_header head(block);
  if(head.type == DIRECTORY){
    directory_header dir(block);
    int ret = 1;
    for(int i = 0; (unsigned int)i < dir.children.size(); i++){
      if(dir.children.at(i) != 0){
        block = mMemblockDevice.readBlock(dir.children.at(i));
        unspecified_header head2(block);
        ret += getSize(checkLoc, head2.name);
      }
    }
    return ret;
  }
  if(head.type == FILE){
    return getFileSize(checkLoc);
  }
  return 0;
}

int FileSystem::getFileSize(int loc){
  if(loc == -1){
    return 0;
  }
  Block block = mMemblockDevice.readBlock(loc);
  file_header file(block);
  return 1 + getFileSize(file.CB);
}

int FileSystem::move(int loc, std::string from, std::string to){
  int fromLoc = findByName(loc, from);
  if(fromLoc < 0){
    return fromLoc;
  }
  int toLoc = findDeepest(loc, to);
  if(toLoc < 0){
    return toLoc - 1;
  }
  char* currDir;
  currDir = strtok(&to[0],"/");
  std::string name = "";
  while(currDir != NULL){
    //std::cout << currDir << std::endl;
    name = currDir;
    currDir = strtok(NULL,"/");
  }
  std::cout << "Name: " << name << std::endl;
  editHeader(fromLoc, name);

  Block block = mMemblockDevice.readBlock(fromLoc);

  header* head = new unspecified_header(block);

  if(head->type == DIRECTORY){
    delete head;
    head = new directory_header(block);
  }else if(head->type == FILE){
    delete head;
    head = new file_header(block);
  }
  std::cout << head->parent << " " << toLoc << std::endl;
  int oldParent = head->parent;
  head->parent = toLoc;

  int parent = toLoc;

  std::vector<char> vec;
  head->pack(vec);
  for(int i = vec.size(); i < block_size; i++){
    vec.push_back(0);
  }
  mMemblockDevice.writeBlock(head->block, vec);

  block = mMemblockDevice.readBlock(oldParent);
  directory_header dir(block);
  dir.removeChild(head->block);

  vec.clear();
  dir.pack(vec);
  for(int i = vec.size(); i < block_size; i++){
    vec.push_back(0);
  }
  mMemblockDevice.writeBlock(dir.block, vec);

  block = mMemblockDevice.readBlock(toLoc);
  directory_header dir2(block);
  dir2.addChild(head->block);

  vec.clear();
  dir2.pack(vec);
  for(int i = vec.size(); i < block_size; i++){
    vec.push_back(0);
  }
  mMemblockDevice.writeBlock(dir2.block, vec);

  delete head;
  delete currDir;
  return toLoc;
}

int FileSystem::findDeepest(int loc, std::string path){
    Block block;
    char* currDir;
    currDir = strtok(&path[0],"/");
    if(currDir == NULL){
      return -1;//empty path error
    }
    if(strcmp(currDir,"root") == 0){
      loc = 0;
    }
    while(currDir != NULL){
      Block block;
      block = mMemblockDevice.readBlock(loc);
      unspecified_header USH(block);
      if(strcmp(currDir,"..") == 0){//move up two
        loc = USH.parent;
      }else if(strcmp(currDir,".") == 0){//move up one
        loc = loc;
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
            return loc;//not found, return deepest location
          }
      }
      currDir = strtok(NULL,"/");
    }
    delete currDir;
    return loc;
}
