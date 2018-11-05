//=============================================================
/*
* @file VoltDBPOCClient.h
* @author Dhirendra Singh
* @date Oct-2018
*/
//=============================================================
#ifndef VOLTDB_POC_CLIENT_H
#define VOLTDB_POC_CLIENT_H


#include "VoltDBPOC.h"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
class VoltDBPOCClient
{

public:
//C'Tor
VoltDBPOCClient();
VoltDBPOCClient(size_t i_instances, Mode i_mode, size_t i_loopCount, size_t i_batchCount);

//D'Tor
~VoltDBPOCClient() { }


//Initialize POC client
void init();


//run POC client
void run();


// thread function
int operator() (int i);


private:


const size_t m_instanceCount;
const char m_firstInstance;
const size_t m_loopCount;
const size_t m_batchCount;
const Mode m_mode;


boost::shared_ptr<boost::thread_group> m_spDBHandlers;
boost::barrier m_barrier; //thread barrier (meeting point of all threads)
};


#endif //VOLTDB_POC_CLIENT_H
