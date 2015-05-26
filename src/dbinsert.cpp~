/*
*	Database connection and transaction test file
*
*	Tom O' Mara
*
*	15/05/2015
*
*		%> g++ -o ../bin/dbinsert dbinsert.cpp -l mysqlcppconn
*		%> ./../bin/dbinsert  
*
*/

#include <stdlib.h>
#include <iostream>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;


int main(void)
{
	cout << "Running 'SELECT * FROM ip_address'" << endl;

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

	// Insert data into the database 
	pstmt = con->prepareStatement("INSERT INTO device(mac_address) VALUES (?)");
	for (int i =1; i < 3; i++) { 
		pstmt->setString(1, "xx:xx:xx:xx:xx:xx");
		pstmt->executeUpdate();	
	}	


	pstmt = con->prepareStatement("INSERT INTO ip_address(mac_address, ip_address) VALUES (?, ?)");
	for (int i =1; i < 3; i++) { 
		pstmt->setString(1, "xx:xx:xx:xx:xx:xx");
		pstmt->setString(2, "localhost-test");
		pstmt->executeUpdate();
		
	}
	

	res = stmt->executeQuery("SELECT * from ip_address");

	while (res->next()) {

		cout << "\t... MySQL replies: ";
		
		// Access column data by alias or column name 
		
		cout << res->getString(1) << endl; //	<-- Get column 1 of query results
		cout << res->getString(2) << endl; // 	<-- Get column 2 of query results 

	}
	delete pstmt;
	delete res; 
	delete stmt;
	delete con; 

	} catch (sql::SQLException &e) { 
	
		cout << "# ERR: SQL Exception in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " ( MySQL error code: " << e.getErrorCode(); 
		cout << ", SQLState: " << e.getSQLState() << " )" << endl; 
	
	}	

	cout << endl; 
	
	return EXIT_SUCCESS;

}
