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
  createFile(0, "Garbage");
  std::vector<char> vec;
  for(int i=0;i<800;i++){
    vec.push_back(i);
  }
  //std::cout << "vec: " << vec.size() << std::endl;
  std::cout << appendData(0, "Garbage", vec) << std::endl;

  std::cout << "First empty: " << findEmptyBlock() << std::endl;

  createFile(0, "Lorem");
  std::string test = "Curabitur consequat rutrum massa, nec elementum risus sollicitudin vel. Quisque mi purus, imperdiet at tortor ac, sollicitudin dignissim ex. Nam condimentum enim non sem molestie semper sit amet a ex. Sed quam sem, dapibus eu interdum eget, pulvinar id sapien. Morbi sit amet justo ac erat efficitur commodo eget id libero. Fusce aliquet lacus non varius luctus. Mauris lobortis nisl sit amet facilisis volutpat. Integer posuere ullamcorper dui. Morbi quis scelerisque risus. Proin tincidunt, tellus quis fermentum finibus, nunc justo tempor neque, non congue tortor tellus at neque. Vivamus scelerisque tortor quis lobortis convallis. Integer vel massa posuere urna viverra pretium in eu eros. Quisque hendrerit quam non turpis congue vestibulum. Nam maximus ligula metus, sit amet sagittis mi mollis ac. Donec at rutrum quam, nec blandit libero. ";

  std::vector<char> to_append(test.begin(), test.end());
  to_append.push_back('\0');
  std::cout << appendData(0, "Lorem", to_append) << std::endl;
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
  file_header dir(parent, block, name);

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
  file_header file(block);
  for(unsigned int i=0;i<content.size();i++){
    file.content.push_back(content.at(i));
  }
  std::vector<char> vec;
  int overflow = file.pack(vec);
  while(vec.size() < block_size){
    vec.push_back(0);
  }
  std::cout << "Write status: " << mMemblockDevice.writeBlock(loc, vec) << std::endl;
  std::cout << "Overflow: " << overflow << std::endl;


  Block primaryBlock = mMemblockDevice.readBlock(loc);
  Block secondaryBlock;
  int file_overflows = 1;
  while(overflow != content.size()){
    std::cout << file_overflows << std::endl;
    file_header parentFile(primaryBlock);
    std::cout << overflow << std::endl;
    int newLoc = createFile(-1, parentFile.name);

    secondaryBlock = mMemblockDevice.readBlock(newLoc);
    file_header childFile(secondaryBlock);
    parentFile.CB = childFile.block;

    for(unsigned int i = overflow; i < content.size();i++){
      childFile.content.push_back(content.at(i));
    }

    std::vector<char> parentVector;
    parentFile.pack(parentVector);
    while(parentVector.size() < block_size){
      parentVector.push_back(0);
    }
    mMemblockDevice.writeBlock(parentFile.block, parentVector);

    std::vector<char> childVector;
    overflow += childFile.pack(childVector);
    while(childVector.size() < block_size){
      childVector.push_back(0);
    }
    mMemblockDevice.writeBlock(childFile.block, childVector);

    Block primaryBlock = mMemblockDevice.readBlock(childFile.block);

    file_overflows++;
  }
  //do overflow
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
  std::cout << "next: " << next << std::endl;
  bool running = true;
  do{
    for(unsigned int i = 0; i < file->content.size(); i++){
      ss << file->content.at(i);
    }
    ss << "\n";
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
