#include "myDeco.h"
#include <iostream>

using namespace std;

DecoderHash::DecoderHash()
{
	size = 0;
	for (int i = 0; i < 256; i++) {
		char a = i;
		string b = "";
		b += a;
		hashList.push_back(b);
		size++;
	}
}

bool DecoderHash::isInHash(int x) // return false if it is not in hash.
{
	if (x > size - 1)
		return false;
	else
		return true;
}

void DecoderHash::addToHash(string ch)
{
	if (size < 4096) {
		hashList.push_back(ch);
		size++;
	}
}

void DecoderHash::printVector()
{
	for (int i = 0; i < size; i++) {
		cout <<i << "="<< hashList[i] << endl;
	}
	cout << endl;
}

int DecoderHash::getSize()
{
	return size;
}

string DecoderHash::getElementAtIndex(int x)
{
	return hashList[x];
}
