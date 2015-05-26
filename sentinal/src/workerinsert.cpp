/*
*	Test worker. Pipeline 2/3
*	
*	Worker application: receives a message, sleeps for that number of seconds, and then signals that its finished:
*
*	connects PULL socket to tcp://localhost:5557
*	collects workloads from ventilator via that socket
*	connects PUSH socket to tcp://localhost:5558
*	sends results to sink via that socket
*	Tom O' Mara
*	Help source http://zguide.zeromq.org/cpp:taskwork
*
*   Comile and run: (cd into correct directory)
*		   (compiler) -o (outputfile) (file-to-comile) -I (path of libary include folder) -lzmq
*		%> g++ -o ../bin/workerinsert workerinsert.cpp -I ../lib/zeromq-4.0.5/include -lzmq -l mysqlcppconn
*		%> ./../bin/workerinsert
*/

#include "../lib/zeromq-4.0.5/include/zhelpers.hpp"
#include "../include/split.h"

#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;


#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>



int main()
{
	zmq::context_t context(1);

	//	Socket to receive messages on
	zmq::socket_t receiver(context, ZMQ_PULL);
	receiver.connect("tcp://127.0.0.1:5557");
	
	//	Socket to send messages to
	zmq::socket_t sender(context, ZMQ_PUSH);
	sender.connect("tcp://127.0.0.1:5558");

	//	Process tasks forever

	while(1){
	
		zmq::message_t message; 

		std::string data;
	
		receiver.recv(&message);

		// 	Put message data into variable
		std::istringstream iss(static_cast<char *>(message.data()));
		iss >> data;

		// ** DO SOMETHING WITH THE VARIABLE! **
		

		// Show contents of message
		std::cout << "Data value:" << data << std::endl;

		// Tokenize message into variable 
		
		vector<string> datavec = split(data, ','); 


/*************************************************
*	CONSTRUCTION AREA A END
*
*		Insert data into database
*
*
***********************************************/
		try {
			sql::Driver *driver;
			sql::Connection *con;
			sql::Statement *stmt;
			sql::ResultSet *res; 
			sql::PreparedStatement *pstmt;
	
		// Create a connection
		driver = get_driver_instance(); 
		con = driver->connect("tcp://127.0.0.1:3306", "root", "password");
	
		// Connect to the MySQL test database 
		con->setSchema("network_database"); 
		stmt = con->createStatement();
		
		// Insert first value of vector (MAC) into our database
		pstmt = con->prepareStatement("INSERT INTO device(mac_address) VALUES (?)");
		pstmt->setString(1, datavec.front()); 
		pstmt->executeUpdate();

		cout << "Device Logged" << endl;		// --> Debug

		// Create iterator
	//	vector<string>::iterator itr; 

		// Iterate over every token within data message
		pstmt = con->prepareStatement("INSERT INTO ip_address(mac_address, ip_address) VALUES (?, ?)");
		for ( int i=0; i < datavec.size() ; i++ ) {

			pstmt->setString(1, datavec.at(i));
			pstmt->setString(2, datavec.at(i+1));
			pstmt->executeUpdate();
			break;
		
		}
		
		cout << "IP Logged" << endl;			// --> Debug
		
		delete con;
		delete stmt;
		delete res;
		delete pstmt;

		

		} catch (sql::SQLException &e) { 
	
			cout << "# ERR: SQL Exception in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " ( MySQL error code: " << e.getErrorCode(); 
			cout << ", SQLState: " << e.getSQLState() << " )" << endl; 
	
		}

			

		cout << endl; 
	


/*************************************************
*	CONSTRUCTION AREA B START
*
*
*
*
***********************************************/		


		//	Send success/failure results to sink
		message.rebuild();
		sender.send(message);

		//	Process indicator for viewer
		std::cout << "." << std::flush;
		
 	
	}	
	return 0;
}

