//=============================================================
/*
* @file VoltDBConnection.h
* @author Dhirendra Singh
* @date Oct-2018
*/
//=============================================================
#ifndef VOLTDB_CONNECTION_H
#define VOLTDB_CONNECTION_H


#include "VoltDBPOC.h"
#include "Client.h"
#include "ProcedureCallback.hpp"
#include <boost/thread.hpp>


extern boost::thread_specific_ptr<char> myInstance;
class VoltDBConnection : public voltdb::ProcedureCallback
{
public:
//C'Tor
VoltDBConnection();


VoltDBConnection(std::string& i_dbHostName, unsigned short i_dbPort);


//D'Tor
~VoltDBConnection() { }


//connect to database
void connect();
//Set the partition IDs-DJ
void setIDS();

//pre transaction processing work method
void prepareEnv();


//run transaction processing loop
void runBatch(Mode i_mode, int i_loopCount = 1, int i_batchCount = 1);


//async mode call back method
bool callback(voltdb::InvocationResponse response) throw (voltdb::Exception);


private:


//fill stored procedures parameter type vector.
void setPartitionIDSParamTypes(); //-DJ
void setOrderInsParamTypes();
void setOrderUpdParamTypes();
void setFAParamTypes();
void setFDParamTypes();
void setFEParamTypes();
void insertPartitionSet(); //-DJ insert partition set
//insert to order table
void insertOrder(std::string& i_orderID, Mode i_mode);
//update order
void updateOrder(std::string& i_orderID, Mode i_mode);
//insert to fill detail table
void insertFillDetail(std::string& i_executionID, Mode i_mode);
//insert to fill allocation table
void insertFillAllocation(std::string& i_fillID, Mode i_mode);


static boost::posix_time::ptime epoch;
static std::string today;


const std::string m_dbHostName; //db host name
const unsigned short m_dbPort; //server port


voltdb::Client m_client; // interface to voltdb connection object

std::vector<voltdb::Parameter> m_PartitionSetParamTypes; // parameter type list of insert to PARTITIONSET procedure-DJ
std::vector<voltdb::Parameter> m_PartitionSetInsParamTypes; //Set InsertPAramters -DJ
std::vector<voltdb::Parameter> m_orderInsParamTypes; // parameter type list of insert to order table stored procedure
std::vector<voltdb::Parameter> m_orderUpdParamTypes; // parameter type list of update order stored procedure
std::vector<voltdb::Parameter> m_faParamTypes; // parameter type list of insert to fill allocation table stored procedure
std::vector<voltdb::Parameter> m_fdParamTypes; // parameter type list
std::vector<voltdb::Parameter> m_feParamTypes; // parameter type list of insert to fill exception table stored procedure

boost::shared_ptr<voltdb::Procedure> m_setPartitionIDs;//Insert to PartitionSET table stored procedure -DJ
boost::shared_ptr<voltdb::Procedure> m_orderInsProcedure; // insert to order table stored procedure
boost::shared_ptr<voltdb::Procedure> m_orderUpdProcedure; // update order stored procedure
boost::shared_ptr<voltdb::Procedure> m_faInsProcedure; // insert to fill allocation stored procedure
boost::shared_ptr<voltdb::Procedure> m_fdInsProcedure; // insert to fill detail stored procedure
boost::shared_ptr<voltdb::Procedure> m_feInsProcedure; // insert to fill exception stored procedure
};


#endif //VOLTDB_CONNECTION_H
