#include "MyHash.h"
#include<iostream>
#include<unordered_map>

using namespace std;


HashTable::HashTable()
{
	size = 0;
	for (int i = 0; i < 256; i++) {
		char a = i;
		string b = "";
		b += a;
		table[b] = i;
		size++;
	}

}

void HashTable::printTable() {
	for (auto x : table)
		cout << x.first << " " << x.second << endl;

}

bool HashTable::isInTable(string ch)
{
	if (table.find(ch) != table.end())
		return true;
	else
		return false;
}

bool HashTable::addElement(string ch, int x) // returns false if element insertion fails due to max num of elements.
{
	if (size < 4096) {
		table[ch] = x;
		size++;
		return true;
	}
	return false;
}

int HashTable::getTableSize()
{
	return size;
}

int HashTable::getNum(string x)
{
	return table[x];
}




