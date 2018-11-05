//=============================================================
/*
* @file VoltDBPOCClient.cpp
* @author Dhirendra Singh
* @date Oct-2018
*/
//=============================================================
#include <iostream>
#include "VoltDBPOCClient.h"

#include "VoltDBConnection.h"


VoltDBPOCClient::VoltDBPOCClient()
:m_instanceCount(1),
m_firstInstance('A'),
m_loopCount(1),
m_batchCount(1),
m_mode(SYNC),
m_spDBHandlers(new boost::thread_group),
m_barrier(m_instanceCount)
{ }
VoltDBPOCClient::VoltDBPOCClient(size_t i_instances, Mode i_mode, size_t i_loopCount, size_t i_batchCount)
:m_instanceCount(i_instances),
m_firstInstance('A'),
m_loopCount(i_loopCount),
m_batchCount(i_batchCount),
m_mode(i_mode),
m_spDBHandlers(new boost::thread_group),
m_barrier(m_instanceCount)
{ }


//Initialize POC client. create a thread to handle each instance.
void VoltDBPOCClient::init()
{
for(int i = 0; i < m_instanceCount; i++) {
boost::thread* t = new boost::thread(boost::ref(*this), i);
(*m_spDBHandlers).add_thread(t);
//m_spDBHandlers->create_thread(boost::ref(*this), i);
}
}
void VoltDBPOCClient::run()
{
//wait for the threads to finish the work
m_spDBHandlers->join_all();
}
// thread function
int VoltDBPOCClient::operator() (int i)
{
//Get my instance
myInstance.reset(new char);
*myInstance = m_firstInstance + i;


try {
VoltDBConnection dbConnection;
//make connection to database.
dbConnection.connect();
//Do pre transaction processing work here. initializing stored procedure work etc...
dbConnection.prepareEnv();
//wait for all threads to reach this point. To make sure all threads start inserting/updating at the same time.
m_barrier.wait();
//run the transaction processing loop
dbConnection.runBatch(m_mode, m_loopCount, m_batchCount);
}
catch (voltdb::ConnectException& e) {
std::cerr << "Error: " << e.what() << std::endl;
return -1;
}
catch (voltdb::LibEventException& e) {
std::cerr << "Error: " << e.what() << std::endl;
return -1;
}
catch (voltdb::Exception& e) {
std::cerr << "Error: " << e.what() << std::endl;
return -1;
}
catch (std::exception& e) {
std::cerr << "Error: " << e.what() << std::endl;
return -1;
}
catch(...) {
std::cerr << "Unknown Exception." << std::endl;
return -1;
}

return 0;
}
