#include "HDD.h"
#include "SSDException.h"
#include <iostream>
using namespace std;

HDD::HDD(int numSectors) : StorageDevice("HDD", numSectors) {
    sectors.resize(numSectors, "");
    occupied.resize(numSectors, false);
}

bool HDD::write(int logicalAddr, const string &data) {
    if (logicalAddr < 0 || logicalAddr >= totalCapacity) {
        throw InvalidAddressException(logicalAddr);
    }
    sectors[logicalAddr] = data;
    occupied[logicalAddr] = true;
    return true;
}

string HDD::read(int logicalAddr) {
    if (logicalAddr < 0 || logicalAddr >= totalCapacity) {
        throw InvalidAddressException(logicalAddr);
    }
    if (!occupied[logicalAddr]) {
        throw EmptyReadException();
    }
    return sectors[logicalAddr];
}

bool HDD::erase(int blockId) {
    if (blockId < 0 || blockId >= totalCapacity) return false;
    sectors[blockId] = "";
    occupied[blockId] = false;
    return true;
}

void HDD::display() const {
    int used = 0;
    for (int i = 0; i < (int)occupied.size(); i++) {
        if (occupied[i]) used++;
    }
    cout << "\n----- HDD Status -----" << endl;
    cout << "Total sectors : " << totalCapacity << endl;
    cout << "Used sectors  : " << used << endl;
    cout << "Free sectors  : " << (totalCapacity - used) << endl;
    cout << "(HDD uses direct sector addressing - no wear leveling, no block erase unit)" << endl;
}
