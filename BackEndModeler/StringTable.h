#ifndef _STRING_TABLE_H_
#define _STRING_TABLE_H_

#include <cstdio>
#include "pin.H"

/*
Note: We never remove a string so we don't need to worry about tombstones
*/
struct HashEntry {
	HashEntry() : empty(true), key(0), pos(0) {}
	bool empty;
	UINT32 key;
	UINT32 pos;
};

//Note:MapCapacity has to be a power of 2
class StringTable {
public:
    StringTable();
    ~StringTable();
    UINT32 insert(const char *str);
    UINT32 getTotalStrSize() const;
    char *get(UINT32 pos) const;
    void dumpTable(FILE *file) const;
private:
    void reallocStr(int newCapacity);
    void reallocMap(int newCapacity);
    UINT32 hash(const char *str, UINT32 hash, uint32_t i);

    char *strList;
    UINT32 strCapacity;
    UINT32 strSize;
    HashEntry *map;
    UINT32 mapCapacity;
    UINT32 mapSize;
};

#endif
