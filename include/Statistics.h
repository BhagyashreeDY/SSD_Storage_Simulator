#ifndef STATISTICS_H
#define STATISTICS_H

#include <vector>
#include "Block.h"
using namespace std;

class Statistics {
private:
    int totalReads;
    int totalWrites;
    int totalErases;

public:
    Statistics();

    void recordRead();
    void recordWrite();
    void recordErase();
    void reset();

    int getTotalReads() const;
    int getTotalWrites() const;
    int getTotalErases() const;

    void display(const vector<Block> &blocks) const;
};

#endif
