//=============================================================
/*
* @file VoltDBConnection.cpp
* @author Dhirendra Singh
* @date Oct-2018
*/
//=============================================================
#include "VoltDBConnection.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <iostream>
#include "iterator"

#include "Table.h"
#include "TableIterator.h"
#include "Row.hpp"
#include "WireType.h"
#include "Parameter.hpp"
#include "ParameterSet.hpp"

boost::thread_specific_ptr<char> myInstance;

boost::posix_time::ptime VoltDBConnection::epoch(boost::posix_time::time_from_string("1970-01-01 00:00:00.000000"));
std::string VoltDBConnection::today(boost::gregorian::to_iso_string(boost::gregorian::day_clock::local_day()));


VoltDBConnection::VoltDBConnection()
:m_dbHostName("localhost"),
m_dbPort(21212),
m_client(voltdb::Client::create())
{ }


VoltDBConnection::VoltDBConnection(std::string& i_dbHostName, unsigned short i_dbPort)
:m_dbHostName(i_dbHostName),
m_dbPort(i_dbPort),
m_client(voltdb::Client::create())
{ }
void VoltDBConnection::connect()
{
// connect to the database.
//voltdb::ClientConfig config("", "");
//m_client = voltdb::Client::create(config);
m_client.createConnection(m_dbHostName, m_dbPort);
std::cout << "Instance " << *myInstance << ": Successfully connected to DB @" << m_dbHostName.c_str() << ":" << m_dbPort << std::endl;
}


void VoltDBConnection::setIDS(){
	setPartitionIDSParamTypes();
	m_setPartitionIDs.reset(new voltdb::Procedure("@GetPartitionKeys,INTEGER"));
}

//Do pre transaction processing work
void VoltDBConnection::prepareEnv()
{
setOrderInsParamTypes();
setOrderUpdParamTypes();
setFAParamTypes();
setFDParamTypes();
setFEParamTypes();


m_orderInsProcedure.reset(new voltdb::Procedure("AGORAORDER.insert", m_orderInsParamTypes));
m_orderUpdProcedure.reset(new voltdb::Procedure("AGORAORDER.update", m_orderUpdParamTypes));
m_faInsProcedure.reset(new voltdb::Procedure("FILLALLOCATIONS.insert", m_faParamTypes));
m_fdInsProcedure.reset(new voltdb::Procedure("FILLDETAILS.insert", m_fdParamTypes));
m_feInsProcedure.reset(new voltdb::Procedure("FILLEXCEPTIONS.insert", m_feParamTypes));
}
//async mode call back method
bool VoltDBConnection::callback(voltdb::InvocationResponse i_response) throw (voltdb::Exception)
{
//Print the error response if there was a problem
if (i_response.failure()) {
std::cout << i_response.toString();
return false;
}
return true;
}
void VoltDBConnection::runBatch(Mode i_mode, int i_loopCount, int i_batchCount)
{
std::cout << "Instance " << *myInstance << ": Starting batch run." << std::endl;
boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::universal_time();
unsigned int batchCount = 0;


for(int i = 0; i < i_loopCount; i++) {
std::ostringstream count;
count << std::setw(15) << std::setfill('0') << i;
std::string id = *myInstance + count.str();

insertOrder(id, i_mode); //insert order
//updateOrder(id, i_mode); //update inserted order
insertFillDetail(id, i_mode); //insert fill detail
insertFillAllocation(id, i_mode); //insert fill allocation

++batchCount;
	if ((i_mode == ASYNC) && (batchCount == i_batchCount)) {
	m_client.runOnce();
	batchCount = 0;
	}
}
if (i_mode == ASYNC) {
m_client.run();
while (!m_client.drain()) { }
}


boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::universal_time();
boost::posix_time::time_duration td = t2 - t1;
long millisecDuration = td.total_milliseconds();
long secDuration = td.total_seconds();
std::cout << "Instance " << *myInstance << ": Finished batch run. Duration = " << millisecDuration << "(ms)/" << secDuration << "(s)" << std::endl;
}
void VoltDBConnection::setPartitionIDSParamTypes() { //-DJ
	m_PartitionSetInsParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_INTEGER));
	m_PartitionSetInsParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_INTEGER)); //AverageFxRate // for setOrderInsParamTypes
////——Do this for the remaining params
}


void VoltDBConnection::setOrderInsParamTypes() {
	m_orderInsParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_STRING));
m_orderInsParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_STRING)); //AverageFxRate // for setOrderInsParamTypes
//——Do this for the remaining params
}

void VoltDBConnection::setOrderUpdParamTypes() {
	m_orderUpdParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_STRING));
m_orderUpdParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_STRING)); //AverageFxRate
//—— Do this for the remaining params
}
void VoltDBConnection::setFAParamTypes()
{
	m_faParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_STRING));
m_faParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_STRING)); //AllocQuantity
//—— Do this for the remaining params
}
void VoltDBConnection::setFDParamTypes()
{
	m_fdParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_STRING));
m_fdParamTypes.push_back(voltdb::Parameter(voltdb::WIRE_TYPE_STRING)); //AllocQuantity
//—— Do this for the remaining params
}

void VoltDBConnection::setFEParamTypes()
{
//

}
void VoltDBConnection::insertPartitionSet() //-DJ
{

voltdb::ParameterSet* params = m_setPartitionIDs->params();
////remaining fields addNull

voltdb::InvocationResponse response;
response=m_client.invoke(*m_setPartitionIDs);

std::vector<voltdb::Table> results = response.results();
voltdb::TableIterator iterator = results[0].iterator();

/* Iterate through the rows */
std::string sqlstatement;
while (iterator.hasNext())
{

    voltdb::Row row = iterator.next();
    std::cout << row.getString(0) << ", " << row.getString(1) << std::endl;
   // sqlstatement="INSERT INTO PARTITIONSET values ("+row.getString(0)+","row.getString(1)");"
    voltdb::ParameterSet* params = m_setPartitionIDs->params();
    m_setPartitionIDs.reset(new voltdb::Procedure("PARTITIONSET.insert",m_PartitionSetInsParamTypes));
    int myint1 = atoi(row.getString(0).c_str());
        int myint2 = atoi(row.getString(1).c_str());
    params->addInt32(myint1); //OrderID
    params->addInt32(myint2); //OrderID
    m_client.invoke(*m_orderInsProcedure);
}

}
//Comment out the call to update order in the main thread


void VoltDBConnection::insertOrder(std::string& i_orderID, Mode i_mode)
{
boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
boost::posix_time::time_duration td = now - epoch;
unsigned long microseconds = td.total_microseconds();


voltdb::ParameterSet* params = m_orderInsProcedure->params();
////remaining fields addNull
//
params->addString(i_orderID); //OrderID
std::string instanceID(1, *myInstance);
params->addString(instanceID); //InstanceID

//

if (i_mode == SYNC) {
voltdb::InvocationResponse response;
response = m_client.invoke(*m_orderInsProcedure);
if (response.failure()) {
std::cout << response.toString();
//return -1;
}
}
else {
m_client.invoke(*m_orderInsProcedure, this);
}
}
//Comment out the call to update order in the main thread
void VoltDBConnection::insertFillDetail(std::string& i_executionID, Mode i_mode)
{
boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
boost::posix_time::time_duration td = now - epoch;
unsigned long microseconds = td.total_microseconds();


voltdb::ParameterSet* params = m_fdInsProcedure->params();
params->addString(i_executionID); //ExecutionID
//
//Populate the Instance ID,
std::string instanceID(1, *myInstance);
params->addString(instanceID); //InstanceID
// addNulls for other params

//

if (i_mode == SYNC) {
voltdb::InvocationResponse response;
response = m_client.invoke(*m_fdInsProcedure);
if (response.failure()) {
std::cout << response.toString();
//return -1;
}
}
else {
m_client.invoke(*m_fdInsProcedure, this);
}
}
void VoltDBConnection::insertFillAllocation(std::string& i_fillID, Mode i_mode)
{
boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
boost::posix_time::time_duration td = now - epoch;
unsigned long microseconds = td.total_microseconds();


voltdb::ParameterSet* params = m_faInsProcedure->params();
//
//params->addString(i_fillID); //ExecutionID

//params->addString(i_fillID); //FillID
//
params->addString(i_fillID); //OrderID
std::string instanceID(1, *myInstance);
params->addString(instanceID); //InstanceID

//

if (i_mode == SYNC) {
voltdb::InvocationResponse response;
response = m_client.invoke(*m_faInsProcedure);
if (response.failure()) {
std::cout << response.toString();
//return -1;
}
}
else {
m_client.invoke(*m_faInsProcedure, this);
}
}
