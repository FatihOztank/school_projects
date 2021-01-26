#pragma once

#ifndef _MYDECO_H
#define _MYDECO_H


#include<iostream>
#include<string>
#include<vector>

using namespace std;

class DecoderHash
{
private:
	vector<string> hashList;
	int size;


public:
	DecoderHash();
	bool isInHash(int x);
	void addToHash(string ch);
	void printVector();
	int getSize();
	string getElementAtIndex(int);



};
#endif // !_MYDECO_H

