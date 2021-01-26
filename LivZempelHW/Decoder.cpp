#include<iostream>
#include<string>
#include"myDeco.h"
#include<vector>
#include<fstream>
#include<sstream>

//M. Fatih ÖZTANK 25060
using namespace std;
DecoderHash x;

string LzwDec(vector<int> num) {
	string result = "";

	for (int t = 0; t < num.size(); t++) {
		if (!x.isInHash(num[t])) {
			x.addToHash(x.getElementAtIndex(num[t - 1]) + x.getElementAtIndex(num[t - 1])[0]);
			result += x.getElementAtIndex(num[t]);
		}
		else {
			result += x.getElementAtIndex(num[t]);
			if (t != 0) {
				x.addToHash(x.getElementAtIndex(num[t - 1]) + x.getElementAtIndex(num[t])[0]);
			}
		}
	}
	
	return result;
}


int main() {
	int num;
	vector<int> decoded;
	string filename , result;
	cout << "please enter the name of the file to be decompressed by lzw algorithm: ";
	cin >> filename;
	ifstream file(filename);
	ofstream output("decompressedString.txt");
	while (file >> num) {
		decoded.push_back(num);
	}
	
	

	result = LzwDec(decoded);
	output << result;





	system("pause");
	return 0;
}