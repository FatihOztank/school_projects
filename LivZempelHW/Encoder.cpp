#include<iostream>
#include <string>
#include"MyHash.h"
#include<fstream>
#include<vector>



using namespace std;
// M. Fatih ÖZTANK 25060

// i defined hashtable as a global variable to use in both functions and main
HashTable x;


vector<int>LzwComp(string input) {// returns a vector of numbers for lzw code.  
	string p = ""; // coded part
	string c = ""; // uncoded part
	vector<int> encode;
	int iter = x.getTableSize();

	for (int i = 0; i < input.length(); i++) {
		p += input[i];
		c = p + input[i + 1];

		if (x.isInTable(p) && !x.isInTable(c)) {
			encode.push_back(x.getNum(p));
			x.addElement(c, iter);
			p = "";
			c = "";
			iter++;
		}
	}
	return encode;
}
int main() {
	char ch;
	string encoded, filename;
	cout << "please enter the name of the file to be compressed by lzw algorithm: ";
	cin >> filename;
	ifstream file(filename);
	ofstream output("compressedCode.txt");
	while (!file.eof()) {
		file.get(ch);
		encoded += ch;
	}
	encoded = encoded.substr(0, encoded.length()-1);
	
	
	vector<int> compressedCode = LzwComp(encoded);
	
	for (int a = 0; a < compressedCode.size(); a++) { // write into file.
		output << compressedCode[a] << " ";
	}

	
	
	


	system("pause");
	return 0;
}