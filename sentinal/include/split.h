/*
*		split function test
*
*		%> g++ -o ./../bin/split split.cpp 
*		%> ./../bin/split
*/


#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::getline;
using std::stringstream;

vector<string> &split(const string &str, char delimiter, vector<string> &elems) {
	stringstream ss(str);
	string item;
	while (getline(ss, item, delimiter)) { 
		elems.push_back(item);
	}
	return elems;
}

vector<string> split(const string &str, char delimiter) {
	vector<string> elems;
	split(str, delimiter, elems);
	return elems;
}

/* 	Uncomment to test 

int main(void) {

	vector<string> x = split("one:two::three", ':'); 

	// Iterate over every reply within replies_packet container 
	vector<string>::iterator itr; 
	for ( itr = x.begin() ; itr < x.end() ; itr++ ) {

		string item = *itr; 

		cout << item << endl;

	}

	return 0; 

}

*/
