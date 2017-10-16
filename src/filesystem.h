#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "memblockdevice.h"
#include <sstream>

class FileSystem
{
private:
    MemBlockDevice mMemblockDevice;

    // Here you can add your own data structures

    template<typename T>
    static char* binary_begin(T &obj){
      return reinterpret_cast<char*>(&obj);
    }

    template<typename T>
    static char* binary_end(T &obj){
      return binary_begin(obj) + sizeof(obj);
    }

    static void appendInt(std::vector<char>& vec, int i){
      std::copy(binary_begin(i),binary_end(i),back_inserter(vec));
    }

    static int extractInt(std::vector<char>& vec, int pos){
      char bytes[4];
      for(int i = 0; i < 4; i++){
        bytes[i] = vec.at(i + pos);
      }
      return *((int*)bytes);
    }

    static void appendString(std::vector<char>& vec, std::string s){
      //appendInt(vec, s.length());
      std::copy(s.begin(), s.end(), back_inserter(vec));
    }

    static std::string extractString(std::vector<char>& vec, int length, int pos){
      char* bytes = new char[length + 1];
      for(int i = 0; i < length; i++){
        bytes[i] = vec.at(i + pos);
      }
      bytes[length] = '\0';
      std::string ret = bytes;
      delete[] bytes;
      return ret;
    }

    struct header{

      enum TYPE{
        DIRECTORY, FILE
      };

    public:

      header(char type, int parent, int block, std::string name){
        this->type = type;
        this->parent = parent;
        this->block = block;
        this->name = name;

        std::cout << printHeaderData() << std::endl;
      }

      header(Block block){
        //std::cout << block.size() << std::endl;
        for(int i=0; i < block.size(); i++){
          //std::cout << i << " : " << (int)block[i] << std::endl;
          data.push_back(block[i]);
        }
        //data = block
        unpackHeader(data);

        std::cout << printHeaderData() << std::endl;
      }

      virtual void pack(std::vector<char> &vec) = 0;

      void packHeader(){
        data.push_back(type);
        appendInt(data, parent);
        appendInt(data, block);

        appendInt(data, name.length());
        appendString(data, name);
      }

      void unpackHeader(std::vector<char>& vec){
        type = vec.at(reader); reader++;
        parent = extractInt(vec,reader); reader+=4;
        block = extractInt(vec,5); reader+=4;

        int length = extractInt(vec, 9); reader += 4;
        name = extractString(vec, length, 13); reader += length;
      }

      virtual void unpack() = 0;

      std::string printHeaderData(){
        std::stringstream ss;

        ss << "Type: " << (TYPE)type
        << "\nParent: " << parent
        << "\nBlock: " << block
        << "\nName: " << name;

        return ss.str();
      }

      std::vector<char> data;

      int reader = 0;

      char type;
      int parent;
      int block;
      std::string name;

    };

    struct directory_header : public header{
    public:
      directory_header(int parent, int block, std::string name) : header(DIRECTORY, parent, block, name){
        //creator
        if(parent == block){
          //root
        }
      }

      directory_header(Block block) : header(block){
        //reader
        unpack();
      }

      void pack(std::vector<char> &vec){
        packHeader();
        //pack all files/subfolders here
        for(int i = 0;i < children.size(); i++){
          data.push_back(children.at(i));
        }

        vec = data;
      }

      void unpack(){

      }

      void addChild(char block){
        children.push_back(block);
      }

      std::vector<char> children;
    };

public:
    FileSystem();
    ~FileSystem();

    /* These API functions need to be implemented
	   You are free to specify parameter lists and return values
    */

    std::string getFullName(int block);

    /* This function creates a file in the filesystem */
    void createFile();

    /* Creates a folder in the filesystem */
    void createFolderi(int parent, std::string name);

    /* Removes a file in the filesystem */
    void removeFile();

    /* Removes a folder in the filesystem */
    void removeFolder();

    /* Function will move the current location to a specified location in the filesystem */
    void goToFolder();

    /* This function will get all the files and folders in the specified folder */
    void listDir();

    /* Add your own member-functions if needed */
};

#endif // FILESYSTEM_H
