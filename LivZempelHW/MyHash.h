#pragma once

#ifndef _MYHASH_H
#define _MYHASH_H

#include<iostream>
#include<unordered_map>
#include<string>

using namespace std;

class HashTable
{
private:
	unordered_map<string, int> table;
	int size;


public:
	HashTable(); // CONSTRUCTOR
	void printTable();
	bool isInTable(string ch);
	bool addElement(string ch, int x);
	int getTableSize();
	int getNum(string);



};
#endif // !_MYHASH_H
