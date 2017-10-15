#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "memblockdevice.h"

class FileSystem
{
private:
    MemBlockDevice mMemblockDevice;
    // Here you can add your own data structures

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
        this->name_length= name_length;
      }

      char type;
      int parent;
      int id;
      int block;

      char name_length;
      char* name;

    };

    struct directory_header : public header{
    public:
      directory_header(int parent, int id, int block, std::string name) : header(DIRECTORY, parent, id, block,  name){

      }
    };

public:
    FileSystem();
    ~FileSystem();

    /* These API functions need to be implemented
	   You are free to specify parameter lists and return values
    */

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
