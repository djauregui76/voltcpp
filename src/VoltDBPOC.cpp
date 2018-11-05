//=====================
/*
* @file VoltDBPOC.cpp
* @author Dhirendra Singh
* @date Oct-2018
*/
//=============================================================
#include <iostream>
#include "ace/Get_Opt.h"
#include "VoltDBPOCClient.h"
#include "VoltDBPOC.h"



void printUsage(char* argv[])
{
std::cout << "Usage: " << argv[0]
<< " [-i number of instances]"
<< " [-m mode ]"
<< " [-l loop count ]"
<< " [-b batch count ]"
<< " [-h ]\n"
<< std::endl;


std::cout << "Options :" << "\n"
<< "\t-i: number of instances. 1 assumed if not passed\n"
<< "\t-m: mode(sync or async). sync assumed if not passed\n"
<< "\t-l: loop count. 1 assumed if not passed\n"
<< "\t-b: batch count. 1 assumed if not passed\n"
<< "\t-h: print this usage message and exit."
<< std::endl;
}


int main(int argc, char* argv[])
{
unsigned int loopCount = 1;
unsigned int batchCount = 1;
unsigned int instances = 1;
Mode mode = SYNC;


//process

const char* modeStr = "sync";

instances = atoi("1");
modeStr = "sync";

//mode = SYNC;
mode = ASYNC;

loopCount = atoi("1");
batchCount = atoi("1");


std::cout << "Number of instances = " << instances << std::endl;
std::cout << "Mode = " << modeStr << "(" << mode << ")" << std::endl;
std::cout << "Loop count = " << loopCount << std::endl;
std::cout << "Batch count = " << batchCount << std::endl;


try {
VoltDBPOCClient client(instances, mode, loopCount, batchCount);
client.init();
client.run();
}


catch (std::exception& e) {
std::cerr << "Error: " << e.what() << std::endl;
return -1;
}
catch (...) {
std::cerr << "Unknown Exception." << std::endl;
return -1;
}


return 0;
}
