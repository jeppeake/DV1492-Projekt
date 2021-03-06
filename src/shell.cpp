#include <iostream>
#include <sstream>
#include "filesystem.h"

const int MAXCOMMANDS = 8;
const int NUMAVAILABLECOMMANDS = 15;

std::string availableCommands[NUMAVAILABLECOMMANDS] = {
    "quit","format","ls","create","cat","createImage","restoreImage",
    "rm","cp","append","mv","mkdir","cd","pwd","help"
};

/* Takes usercommand from input and returns number of commands, commands are stored in strArr[] */
int parseCommandString(const std::string &userCommand, std::string strArr[]);
int findCommand(std::string &command);
bool quit();
std::string help();

/* More functions ... */

int main(void) {

	std::string userCommand, commandArr[MAXCOMMANDS];
	std::string user = "Operator";
	std::string currentDir = "root";    // current directory, used for output

  int currBlock = 0;

  FileSystem * FS = 0;

    bool bRun = true;
    FS = new FileSystem();
    do {
        std::cout << user << ":" << currentDir << "$ ";
        getline(std::cin, userCommand);

        int nrOfCommands = parseCommandString(userCommand, commandArr);
        if (nrOfCommands > 0) {

            int cIndex = findCommand(commandArr[0]);
            switch(cIndex) {

			      case 0: //quit
				        bRun = quit();
                if(FS != 0){
                  delete FS;
                }
                break;
            case 1: // format
                std::cout << "Formatting operation starting" << std::endl;
                if(FS != 0){
                  delete FS;
                  std::cout << "Old system deleted" << std::endl;
                }
                FS = new FileSystem();
                break;
            case 2:{ // ls
                if(nrOfCommands == 2){
                  std::string path = commandArr[1];
                  int ret = FS->goToFolder(path, currBlock);
                  //std::cout << ret << std::endl;
                  if(ret >= 0){
                    std::cout << FS->listDir(ret) << std::endl;
                  }else if(ret == -1){
                    std::cout << "Path not found." << std::endl;
                  } else if (ret == -2){
                    std::cout << "Path error, non-directory included." << std::endl;
                  }
                }else if (nrOfCommands == 1){
                  std::cout << FS->listDir(currBlock) << std::endl;
                }else{
                  std::cout << "Command error" << std::endl;
                }
                break;
              }
            case 3: // create
                if(nrOfCommands >= 2){
                  FS->createFile(currBlock, commandArr[1]);
                  std::cout << "Enter data: " << std::endl;
                  std::string data;
                  std::getline(std::cin, data);
                  std::vector<char> to_append(data.begin(), data.end());
                  to_append.push_back('\0');
                  FS->appendData(currBlock, commandArr[1], to_append);
                }else{
                  std::cout << "Missing parameter <name>" << std::endl;
                }
                break;
            case 4: // cat
                if(nrOfCommands >= 2){
                  std::cout << FS->readFile(currBlock, commandArr[1]) << std::endl;
                }else{
                  std::cout << "Missing parameter <name>" << std::endl;
                }
                break;
            case 5: // createImage
                if(nrOfCommands < 2)
                {
                  std::cout << "Missing parameter <real-file>" << std::endl;
                  break;
                }
                FS->saveToFile(commandArr[1]);
                break;
            case 6: // restoreImage
                if(nrOfCommands < 2)
                {
                  std::cout << "Missing parameter <real-file>" << std::endl;
                  break;
                }
                if(FS == 0)
                {
                  FS = new FileSystem();
                  if(!FS->loadFromFile(commandArr[1]))
                    {std::cout << "Could not find file." << std::endl;break;}
                }
                else
                {
                  if(!FS->loadFromFile(commandArr[1]))
                    {std::cout << "Could not find file." << std::endl; break;}
                  else
                    {delete FS; FS = new FileSystem();}
                }
                FS->loadFromFile(commandArr[1]);
                currBlock = 0;
                currentDir = "root";
                std::cout << "Image restored succesfully." << std::endl;
                break;
            case 7: // rm
                if(nrOfCommands < 2)
                {
                  std::cout << "Missing parameter <name>" << std::endl;
                  break;
                }
                FS->removeFile(currBlock,commandArr[1]);
                break;
            case 8:{ // cp
                  if(nrOfCommands < 3){
                    std::cout << "Missing parameters" << std::endl;
                  }
                  int ret = FS->copy(currBlock, commandArr[1], commandArr[2]);
                  if(ret == -1){
                    std::cout << commandArr[1] << " not found." << std::endl;
                  }
                  if(ret == -2){
                    std::cout << commandArr[2] << " not found." << std::endl;
                  }
                  if(ret == -3){
                    std::cout << commandArr[2] << " is not a file." << std::endl;
                  }
                  if(ret == -4){
                    std::cout << "Illegal operation!" << std::endl;
                  }
                  break;
                }
            case 9:{ // append
                  if(nrOfCommands >= 3){
                    std::string appender = FS->readFile(currBlock, commandArr[2]);
                    std::vector<char> to_append(appender.begin(), appender.end());
                    to_append.push_back('\0');
                    int ret = FS->appendData(currBlock, commandArr[1], to_append);
                    if (ret < 0){
                      std::cout << commandArr[1] << " not found." << std::endl;
                    }
                  }else{
                    std::cout << "Missing parameters" << std::endl;
                  }
                }
                break;
            case 10:{ // mv
              if(nrOfCommands >= 3){
                int ret = FS->move(currBlock, commandArr[1], commandArr[2]);
                if(ret == -1){
                  std::cout << commandArr[1] << " not found." << std::endl;
                  break;
                }
                if(ret == -2){
                  std::cout << "Cannot operate on empty argument" << std::endl;
                  break;
                }
                if(ret == -3){
                  std::cout << commandArr[2] << " not found." << std::endl;
                  break;
                }
              }else{
                std::cout << "Missing parameters" << std::endl;
              }
                /*if(nrOfCommands >= 3){
                  if(commandArr[2].find("/") != std::string::npos){//move
                    int ret = FS->copy(currBlock, commandArr[1], commandArr[2]);
                    if(ret == -1){
                      std::cout << commandArr[1] << " not found." << std::endl;
                      break;
                    }
                    if(ret == -2){
                      std::cout << commandArr[2] << " not found." << std::endl;
                      break;
                    }
                    if(ret == -3){
                      std::cout << commandArr[2] << " is not a file." << std::endl;
                      break;
                    }
                    if(ret == -4){
                      std::cout << "Illegal operation!" << std::endl;
                      break;
                    }
                    FS->removeFile(currBlock, commandArr[1]);
                  }else{//rename
                    int to_rename = FS->findByName(currBlock, commandArr[1]);
                    if(to_rename == -1){
                      std::cout << commandArr[1] << " not found." << std::endl;
                      break;
                    }
                    if(to_rename == 0){
                      std::cout << "Can't operate on root!" << std::endl;
                      break;
                    }
                    if(commandArr[2] == "root"){
                      std::cout << "Illegal operation!" << std::endl;
                      break;
                    }
                    FS->editHeader(to_rename, commandArr[2]);
                  }
                }else{
                  std::cout << "Missing parameters" << std::endl;
                }*/
                break;
              }
            case 11: // mkdir
                if(nrOfCommands >= 2){
                  FS->createFolderi(currBlock, commandArr[1]);
                }else{
                  std::cout << "Missing parameter <name>" << std::endl;
                }
                break;
            case 12:{ // cd
                if(nrOfCommands >= 2){
                  std::string name = commandArr[1];
                  int returnCode = FS->goToFolder(name, currBlock);
                  if(returnCode == -2){
                    std::cout << name << " is not a directory." << std::endl;
                    break;
                  }
                  if(returnCode == -1){
                    std::cout << name << " not found." << std::endl;
                    break;
                  }
                  currBlock = returnCode;
                  currentDir = FS->getLocation(currBlock);
                }else{
                  std::cout << "Missing parameter <path>" << std::endl;
                }
                break;
              }
            case 13: // pwd
                std::cout << FS->getLocation(currBlock) << std::endl;
                break;
            case 14: // help
                std::cout << help() << std::endl;
                break;
            default:
                std::cout << "Unknown command: " << commandArr[0] << std::endl;
            }
        }
    } while (bRun == true);

    return 0;
}

int parseCommandString(const std::string &userCommand, std::string strArr[]) {
    std::stringstream ssin(userCommand);
    int counter = 0;
    while (ssin.good() && counter < MAXCOMMANDS) {
        ssin >> strArr[counter];
        counter++;
    }
    if (strArr[0] == "") {
        counter = 0;
    }
    return counter;
}
int findCommand(std::string &command) {
    int index = -1;
    for (int i = 0; i < NUMAVAILABLECOMMANDS && index == -1; ++i) {
        if (command == availableCommands[i]) {
            index = i;
        }
    }
    return index;
}

bool quit() {
	std::cout << "Exiting\n";
	return false;
}

std::string help() {
    std::string helpStr;
    helpStr += "OSD Disk Tool .oO Help Screen Oo.\n";
    helpStr += "-----------------------------------------------------------------------------------\n" ;
    helpStr += "* quit:                             Quit OSD Disk Tool\n";
    helpStr += "* format;                           Formats disk\n";
    helpStr += "* ls     <path>:                    Lists contents of <path>.\n";
    helpStr += "* create <path>:                    Creates a file and stores contents in <path>\n";
    helpStr += "* cat    <path>:                    Dumps contents of <file>.\n";
    helpStr += "* createImage  <real-file>:         Saves disk to <real-file>\n";
    helpStr += "* restoreImage <real-file>:         Reads <real-file> onto disk\n";
    helpStr += "* rm     <file>:                    Removes <file>\n";
    helpStr += "* cp     <source> <destination>:    Copy <source> to <destination>\n";
    helpStr += "* append <source> <destination>:    Appends contents of <source> to <destination>\n";
    helpStr += "* mv     <old-file> <new-file>:     Renames <old-file> to <new-file>\n";
    helpStr += "* mkdir  <directory>:               Creates a new directory called <directory>\n";
    helpStr += "* cd     <directory>:               Changes current working directory to <directory>\n";
    helpStr += "* pwd:                              Get current working directory\n";
    helpStr += "* help:                             Prints this help screen\n";
    return helpStr;
}

/* Insert code for your shell functions and call them from the switch-case */
