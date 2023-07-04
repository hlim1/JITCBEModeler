/**
 * Creates a simple hash map for storing strings. It allows the user to store and find a string, but does 
 * not include the ability to delete a string (to improvie performance and due to laziness). It is set up
 * so that all strings are stored in a single character array, and the index to an individual string is
 * stored in the hash map. This gives us the ability to dump the string table really easily since all we
 * have to do is return the character array of strings. Additionally, collisions are resolved using double-
 * hashing. Since everyting is fairly standard here, I've neglected to include function definitions.
 **/

#include "StringTable.h"
#include <cstring>

StringTable::StringTable() {
	strCapacity = 4096;
	strList = new char[strCapacity];
	strSize = 0;
	mapCapacity = 256;
	map = new HashEntry[mapCapacity];
	mapSize = 0;

	//the empty string has to be at position 0 or we'll end up with issues
	insert("");
}

StringTable::~StringTable() {
	delete[] strList;
	delete[] map;
}

void StringTable::reallocMap(int newCapacity) {
	HashEntry *newMap = new HashEntry[newCapacity];
	for(UINT32 i = 0; i < mapCapacity; i++) {
		UINT32 hashVal = 0;
		UINT32 ind = 0;
		//try to find
		for(uint32_t j = 0;; j++) {
			hashVal = hash(strList + map[i].pos, hashVal, j);
			ind = hashVal & (newCapacity - 1);
			if(newMap[ind].empty == true) {
				//found empty space, go ahead and add
				break;
			}
		}
		newMap[ind].empty = false;
		newMap[ind].key = hashVal;
		newMap[ind].pos = map[i].pos;
	}

	delete[] map;
	map = newMap;
	mapCapacity = newCapacity;
}

void StringTable::reallocStr(int newCapacity) {
	char *newStrList = new char[newCapacity];
	memcpy(newStrList, strList, strSize);
	delete[] strList;
	strList = newStrList;
	strCapacity = newCapacity;
}

/*
	alg taken from: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
*/
UINT32 StringTable::hash(const char *str, UINT32 hash, uint32_t i) {
    hash += i;
	unsigned char *s = (unsigned char *)str;
	while(*s) {
		hash ^= (UINT32)*s++;
		hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
	}
	return hash;
}

UINT32 StringTable::insert(const char *str) {
	UINT32 hashVal = 0;
	UINT32 ind = 0;
	//try to find
	for(uint32_t i = 0;; i++) {
		hashVal = hash(str, hashVal, i);
		ind = hashVal & (mapCapacity - 1);
		if(map[ind].empty == true) {
			//need to insert
			break;
		}
		if(hashVal == map[ind].key) {
			if(strcmp(str, (strList + map[ind].pos)) == 0) {
				return map[ind].pos;
			}
		}
	}

	//insert
	UINT32 newStrlen = strlen(str) + 1;

	//make room if necessary
	if((newStrlen + strSize) > strCapacity) {
		UINT32 newStrCapacity = strCapacity << 2;
		while((newStrlen + strSize) > newStrCapacity) {
			newStrCapacity = newStrCapacity << 2;
		}
		reallocStr(newStrCapacity);
	}
	if(mapSize > (mapCapacity >> 2)) {
		reallocMap(mapCapacity << 2);
	}

	UINT32 pos = strSize;
	map[ind].empty = false;
	map[ind].key = hashVal;
	map[ind].pos = pos;
	memcpy((strList + strSize), str, newStrlen);
	strSize += newStrlen;
    mapSize++;

	return pos;
}

UINT32 StringTable::getTotalStrSize() const {
	return strSize;
}

char *StringTable::get(UINT32 pos) const {
	return strList + pos;
}

void StringTable::dumpTable(FILE *file) const {
	fwrite(strList, sizeof(char), strSize, file);
}

