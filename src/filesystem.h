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
    char* binary_begin(T &obj){
      return reinterpret_cast<char*>(&obj);
    }

    template<typename T>
    char* binary_end(T &obj){
      return binary_begin(obj) + sizeof(obj);
    }

    std::vector<char>& appendInt(std::vector<char>& vec, int i){
      std::copy(binary_begin(i),binary_end(i),back_inserter(vec));
      return vec;
    }

    int extractInt(std::vector<char>& vec, int pos){
      char bytes[4];
      for(int i = 0; i < 4; i++){
        bytes[i] = vec.at(i + pos);
      }
      return *((int*)bytes);
    }

    struct header{

      enum TYPE{
        DIRECTORY, FILE
      };

    public:

      header(char type, int parent, int id, int block, std::string name){
        this->type = type;
        this->parent = parent;
        this->id = id;
        this->block = block;
        this->name = name;

        std::cout << printHeaderData() << std::endl;
      }

      virtual void pack() = 0;

      void packHeader(std::vector<char> &vec){
        vec.push_back(type);

      }

      std::string printHeaderData(){
        std::stringstream ss;

        ss << "Type: " << (int)type
        << "\nParent: " << parent
        << "\nId: " << id
        << "\nBlock: " << block
        << "\nName: " << name;

        return ss.str();
      }

      std::vector<char> data;

      char type;
      int parent;
      int id;
      int block;
      std::string name;

    };

    struct directory_header : public header{
    public:
      directory_header(int parent, int id, int block, std::string name) : header(DIRECTORY, parent, id, block,  name){

      }
      void pack(){

      }
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
    void createFolderi(int parent, int id, int block, std::string name);

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
