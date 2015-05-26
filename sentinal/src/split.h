#include <string>
#include <sstream>
#include <vector>

using std::vector;
using std::string;
using std::getline;

vector<string> &split(const string &str, char delimiter, vector<string> &elems {
	stringstream ss(s);
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

int main(void) {

	vector<string> x = split("one:two::three", ':'); 

	// Iterate over every reply within replies_packet container 
	vector<string>::iterator itr; 
	int counter = 0; 
	for ( itr = x.begin() ; itr < x.end() ; itr++ ) {

		string item = itr; 

		cout << itr << endl;

	}

	return 0; 

}
