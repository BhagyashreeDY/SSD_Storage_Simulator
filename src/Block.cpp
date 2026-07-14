#include "Block.h"
#include <iostream>
using namespace std;

Block::Block() {
    blockId = -1;
    wearCount = 0;
}

Block::Block(int id, int pagesPerBlock) {
    blockId = id;
    wearCount = 0;
    for (int i = 0; i < pagesPerBlock; i++) {
        pages.push_back(Page(i));
    }
}

int Block::findFreePage() const {
    for (int i = 0; i < (int)pages.size(); i++) {
        if (pages[i].isFree()) {
            return i;
        }
    }
    return -1;
}

void Block::eraseBlock() {
    for (int i = 0; i < (int)pages.size(); i++) {
        pages[i].reset();
    }
    wearCount++;
}

double Block::getInvalidRatio() const {
    if (pages.empty()) return 0.0;
    return (double)getInvalidPageCount() / (double)pages.size();
}

int Block::getWearCount() const {
    return wearCount;
}

void Block::setWearCount(int w) {
    wearCount = w;
}

int Block::getBlockId() const {
    return blockId;
}

int Block::getPagesPerBlock() const {
    return (int)pages.size();
}

int Block::getFreePageCount() const {
    int count = 0;
    for (int i = 0; i < (int)pages.size(); i++) {
        if (pages[i].isFree()) count++;
    }
    return count;
}

int Block::getUsedPageCount() const {
    int count = 0;
    for (int i = 0; i < (int)pages.size(); i++) {
        if (pages[i].isValidPage()) count++;
    }
    return count;
}

int Block::getInvalidPageCount() const {
    int count = 0;
    for (int i = 0; i < (int)pages.size(); i++) {
        if (pages[i].isInvalidPage()) count++;
    }
    return count;
}

Page &Block::getPageRef(int idx) {
    return pages[idx];
}

const Page &Block::getPageRef(int idx) const {
    return pages[idx];
}

void Block::displayBlock() const {
    cout << "Block " << blockId << " | wear=" << wearCount
         << " | free=" << getFreePageCount()
         << " | used=" << getUsedPageCount()
         << " | invalid=" << getInvalidPageCount() << endl;
}
