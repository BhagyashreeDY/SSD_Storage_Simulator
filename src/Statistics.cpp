#include "Statistics.h"
#include <iostream>
#include <algorithm>
using namespace std;

Statistics::Statistics() {
    totalReads = 0;
    totalWrites = 0;
    totalErases = 0;
}

void Statistics::recordRead() { totalReads++; }
void Statistics::recordWrite() { totalWrites++; }
void Statistics::recordErase() { totalErases++; }

void Statistics::reset() {
    totalReads = 0;
    totalWrites = 0;
    totalErases = 0;
}

int Statistics::getTotalReads() const { return totalReads; }
int Statistics::getTotalWrites() const { return totalWrites; }
int Statistics::getTotalErases() const { return totalErases; }

void Statistics::display(const vector<Block> &blocks) const {
    int freePages = 0, usedPages = 0, invalidPages = 0, totalWear = 0;

    for (int i = 0; i < (int)blocks.size(); i++) {
        freePages += blocks[i].getFreePageCount();
        usedPages += blocks[i].getUsedPageCount();
        invalidPages += blocks[i].getInvalidPageCount();
        totalWear += blocks[i].getWearCount();
    }

    // sort a copy of the blocks by wear count just to show them worst-first
    // this is where sort() from <algorithm> gets used, on a small
    // vector of (wear, blockId) pairs instead of copying whole Block objects
    vector<pair<int,int>> wearList;
    for (int i = 0; i < (int)blocks.size(); i++) {
        wearList.push_back({blocks[i].getWearCount(), blocks[i].getBlockId()});
    }
    sort(wearList.begin(), wearList.end(), [](pair<int,int> a, pair<int,int> b) {
        return a.first > b.first;
    });

    cout << "\n----- SSD Statistics -----" << endl;
    cout << "Total Reads       : " << totalReads << endl;
    cout << "Total Writes      : " << totalWrites << endl;
    cout << "Total Erase Ops   : " << totalErases << endl;
    cout << "Free Pages        : " << freePages << endl;
    cout << "Used Pages        : " << usedPages << endl;
    cout << "Invalid Pages     : " << invalidPages << endl;
    cout << "Total Wear Count  : " << totalWear << endl;

    cout << "Blocks sorted by wear (highest first): ";
    for (int i = 0; i < (int)wearList.size(); i++) {
        cout << "B" << wearList[i].second << "(" << wearList[i].first << ") ";
    }
    cout << endl;
}
