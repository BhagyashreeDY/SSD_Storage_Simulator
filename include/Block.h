#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include "Page.h"
using namespace std;

class Block {
private:
    int blockId;
    vector<Page> pages;
    int wearCount;

public:
    Block();
    Block(int id, int pagesPerBlock);

    int findFreePage() const;   // returns index of first free page, -1 if none
    void eraseBlock();          // resets every page, increases wear count

    double getInvalidRatio() const;
    int getWearCount() const;
    void setWearCount(int w);   // only used when restoring a saved state
    int getBlockId() const;
    int getPagesPerBlock() const;

    int getFreePageCount() const;
    int getUsedPageCount() const;
    int getInvalidPageCount() const;

    Page &getPageRef(int idx);
    const Page &getPageRef(int idx) const;

    void displayBlock() const;
};

#endif
