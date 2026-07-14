#ifndef SSD_H
#define SSD_H

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include "StorageDevice.h"
#include "Block.h"
#include "Statistics.h"
#include "Cache.h"
using namespace std;

class SSD : public StorageDevice {
private:
    vector<Block> blocks;
    unordered_map<int, int> lbaToPhysical;   // logical address -> encoded (blockIdx*pagesPerBlock + pageIdx)
    queue<int> pendingWrites;                // logical addresses waiting to be flushed
    unordered_map<int, string> pendingData;  // data paired with the queued addresses above

    // min-heap on (wearCount, blockId) - always gives the least worn
    // block first. entries go stale whenever a block gets erased since
    // its wear count changes, so we check the stored wear against the
    // block's current wear count when popping and just discard stale ones.
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> wearMinHeap;

    int pagesPerBlock;
    int totalBlocks;
    Statistics stats;
    Cache cache;

    int allocateFreePage();  // returns encoded location, -1 if SSD is full
    int encodeLocation(int blockIdx, int pageIdx) const;
    pair<int,int> decodeLocation(int loc) const;

public:
    SSD();
    SSD(int numBlocks, int pagesPerBlockIn);
    SSD(const SSD &other);
    ~SSD();

    bool write(int logicalAddr, const string &data) override;
    string read(int logicalAddr) override;
    bool erase(int blockId) override;
    void display() const override;

    bool updateData(int logicalAddr, const string &newData);
    bool deleteData(int logicalAddr);
    void runGarbageCollection();

    void queueWrite(int logicalAddr, const string &data);
    void processPendingWrites();

    void saveState(const string &filename);
    void loadState(const string &filename);

    void showStatistics() const;

    int getTotalBlocks() const;
    int getPagesPerBlock() const;
};

#endif
