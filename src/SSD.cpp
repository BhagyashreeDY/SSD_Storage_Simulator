#include "SSD.h"
#include "SSDException.h"
#include <iostream>
#include <fstream>
using namespace std;

// small helpers for writing/reading variable length strings in the
// binary save file - length first, then raw characters
static void writeStringBin(ofstream &out, const string &s) {
    int len = (int)s.size();
    out.write(reinterpret_cast<const char *>(&len), sizeof(len));
    if (len > 0) out.write(s.c_str(), len);
}

static string readStringBin(ifstream &in) {
    int len = 0;
    in.read(reinterpret_cast<char *>(&len), sizeof(len));
    string s(len, ' ');
    if (len > 0) in.read(&s[0], len);
    return s;
}

SSD::SSD() : StorageDevice("SSD", 0) {
    pagesPerBlock = 0;
    totalBlocks = 0;
}

SSD::SSD(int numBlocks, int pagesPerBlockIn)
    : StorageDevice("SSD", numBlocks * pagesPerBlockIn) {
    totalBlocks = numBlocks;
    pagesPerBlock = pagesPerBlockIn;

    for (int i = 0; i < numBlocks; i++) {
        blocks.push_back(Block(i, pagesPerBlock));
        wearMinHeap.push({0, i});
    }
}

SSD::SSD(const SSD &other) : StorageDevice(other.deviceName, other.totalCapacity) {
    blocks = other.blocks;
    lbaToPhysical = other.lbaToPhysical;
    pendingWrites = other.pendingWrites;
    pendingData = other.pendingData;
    wearMinHeap = other.wearMinHeap;
    pagesPerBlock = other.pagesPerBlock;
    totalBlocks = other.totalBlocks;
    stats = other.stats;
    cache = other.cache;
}

SSD::~SSD() {
    // all members are STL containers (vector, unordered_map, queue,
    // priority_queue) and clean up their own heap allocations
    // automatically, so there's nothing to free manually here
}

int SSD::encodeLocation(int blockIdx, int pageIdx) const {
    return blockIdx * pagesPerBlock + pageIdx;
}

pair<int, int> SSD::decodeLocation(int loc) const {
    return {loc / pagesPerBlock, loc % pagesPerBlock};
}

int SSD::allocateFreePage() {
    while (!wearMinHeap.empty()) {
        pair<int, int> top = wearMinHeap.top();
        int wear = top.first;
        int blockId = top.second;
        wearMinHeap.pop();

        // wear count no longer matches -> this entry is stale (the
        // block got erased since this entry was pushed), discard it
        if (blocks[blockId].getWearCount() != wear) {
            continue;
        }

        int pageIdx = blocks[blockId].findFreePage();
        if (pageIdx != -1) {
            // still has room, so it stays eligible for future allocations
            wearMinHeap.push({wear, blockId});
            return encodeLocation(blockId, pageIdx);
        }
        // block is full - leave it out of the heap until it gets erased
        // and its wear count (and heap entry) changes
    }
    return -1;
}

bool SSD::write(int logicalAddr, const string &data) {
    if (logicalAddr < 0) {
        throw InvalidAddressException(logicalAddr);
    }

    auto it = lbaToPhysical.find(logicalAddr);
    if (it != lbaToPhysical.end()) {
        pair<int, int> old = decodeLocation(it->second);
        blocks[old.first].getPageRef(old.second).invalidate();
        lbaToPhysical.erase(it);
        cache.invalidate(logicalAddr);
    }

    int loc = allocateFreePage();
    if (loc == -1) {
        runGarbageCollection();
        loc = allocateFreePage();
        if (loc == -1) {
            throw SSDFullException();
        }
    }

    pair<int, int> pos = decodeLocation(loc);
    blocks[pos.first].getPageRef(pos.second).writeData(data, logicalAddr);
    lbaToPhysical[logicalAddr] = loc;
    cache.put(logicalAddr, data);
    stats.recordWrite();
    return true;
}

string SSD::read(int logicalAddr) {
    auto it = lbaToPhysical.find(logicalAddr);
    if (it == lbaToPhysical.end()) {
        throw InvalidAddressException(logicalAddr);
    }

    string cached;
    if (cache.get(logicalAddr, cached)) {
        stats.recordRead();
        return cached;
    }

    pair<int, int> pos = decodeLocation(it->second);
    string result = blocks[pos.first].getPageRef(pos.second).getData();
    cache.put(logicalAddr, result);
    stats.recordRead();
    return result;
}

bool SSD::erase(int blockId) {
    if (blockId < 0 || blockId >= totalBlocks) {
        return false;
    }

    // drop any mappings pointing into this block since its pages are
    // about to be wiped clean
    vector<int> toRemove;
    for (auto &entry : lbaToPhysical) {
        pair<int, int> pos = decodeLocation(entry.second);
        if (pos.first == blockId) {
            toRemove.push_back(entry.first);
        }
    }
    for (int lba : toRemove) {
        lbaToPhysical.erase(lba);
        cache.invalidate(lba);
    }

    blocks[blockId].eraseBlock();
    wearMinHeap.push({blocks[blockId].getWearCount(), blockId});
    stats.recordErase();
    return true;
}

void SSD::display() const {
    cout << "\n----- SSD Status -----" << endl;
    cout << "Total blocks    : " << totalBlocks << endl;
    cout << "Pages per block : " << pagesPerBlock << endl;
    for (int i = 0; i < totalBlocks; i++) {
        blocks[i].displayBlock();
    }
}

bool SSD::updateData(int logicalAddr, const string &newData) {
    if (lbaToPhysical.find(logicalAddr) == lbaToPhysical.end()) {
        throw InvalidAddressException(logicalAddr);
    }
    // reuses the same invalidate-old-page + allocate-new-page path as
    // write() - an update is really just a write to an address that
    // already exists
    return write(logicalAddr, newData);
}

bool SSD::deleteData(int logicalAddr) {
    auto it = lbaToPhysical.find(logicalAddr);
    if (it == lbaToPhysical.end()) {
        return false;
    }
    pair<int, int> pos = decodeLocation(it->second);
    blocks[pos.first].getPageRef(pos.second).invalidate();
    lbaToPhysical.erase(it);
    cache.invalidate(logicalAddr);
    return true;
}

void SSD::runGarbageCollection() {
    for (int i = 0; i < totalBlocks; i++) {
        if (blocks[i].getInvalidRatio() <= 0.5) {
            continue;
        }

        bool migrationFailed = false;

        for (int p = 0; p < blocks[i].getPagesPerBlock(); p++) {
            Page &pg = blocks[i].getPageRef(p);
            if (!pg.isValidPage()) continue;

            string d = pg.getData();
            int lba = pg.getMappedLBA();
            int newLoc = -1;

            // find a free page in some other block to move this data
            // into - not going through the wear-leveling heap here to
            // keep migration simple and avoid re-entering allocateFreePage
            // while we're already in the middle of a GC pass
            for (int j = 0; j < totalBlocks && newLoc == -1; j++) {
                if (j == i) continue;
                int freeIdx = blocks[j].findFreePage();
                if (freeIdx != -1) {
                    newLoc = encodeLocation(j, freeIdx);
                    blocks[j].getPageRef(freeIdx).writeData(d, lba);
                }
            }

            if (newLoc == -1) {
                migrationFailed = true;
                break;
            }
            lbaToPhysical[lba] = newLoc;
        }

        if (!migrationFailed) {
            blocks[i].eraseBlock();
            wearMinHeap.push({blocks[i].getWearCount(), i});
            stats.recordErase();
        }
    }
}

void SSD::queueWrite(int logicalAddr, const string &data) {
    pendingWrites.push(logicalAddr);
    pendingData[logicalAddr] = data;
}

void SSD::processPendingWrites() {
    while (!pendingWrites.empty()) {
        int addr = pendingWrites.front();
        pendingWrites.pop();
        auto it = pendingData.find(addr);
        if (it != pendingData.end()) {
            write(addr, it->second);
            pendingData.erase(it);
        }
    }
}

void SSD::saveState(const string &filename) {
    ofstream out(filename, ios::binary);
    if (!out.is_open()) {
        cout << "Could not open file for saving: " << filename << endl;
        return;
    }

    out.write(reinterpret_cast<const char *>(&totalBlocks), sizeof(totalBlocks));
    out.write(reinterpret_cast<const char *>(&pagesPerBlock), sizeof(pagesPerBlock));

    for (int i = 0; i < totalBlocks; i++) {
        int wear = blocks[i].getWearCount();
        out.write(reinterpret_cast<const char *>(&wear), sizeof(wear));

        for (int p = 0; p < pagesPerBlock; p++) {
            const Page &pg = blocks[i].getPageRef(p);
            int stateVal = pg.isFree() ? 0 : (pg.isValidPage() ? 1 : 2);
            int lba = pg.getMappedLBA();
            out.write(reinterpret_cast<const char *>(&stateVal), sizeof(stateVal));
            out.write(reinterpret_cast<const char *>(&lba), sizeof(lba));
            writeStringBin(out, pg.getData());
        }
    }

    int mapSize = (int)lbaToPhysical.size();
    out.write(reinterpret_cast<const char *>(&mapSize), sizeof(mapSize));
    for (auto &entry : lbaToPhysical) {
        out.write(reinterpret_cast<const char *>(&entry.first), sizeof(entry.first));
        out.write(reinterpret_cast<const char *>(&entry.second), sizeof(entry.second));
    }

    int reads = stats.getTotalReads();
    int writes = stats.getTotalWrites();
    int erases = stats.getTotalErases();
    out.write(reinterpret_cast<const char *>(&reads), sizeof(reads));
    out.write(reinterpret_cast<const char *>(&writes), sizeof(writes));
    out.write(reinterpret_cast<const char *>(&erases), sizeof(erases));

    out.close();
    cout << "SSD state saved to " << filename << endl;
}

void SSD::loadState(const string &filename) {
    ifstream in(filename, ios::binary);
    if (!in.is_open()) {
        cout << "Could not open file for loading: " << filename << endl;
        return;
    }

    int numBlocks = 0, pagesPerBlockIn = 0;
    in.read(reinterpret_cast<char *>(&numBlocks), sizeof(numBlocks));
    in.read(reinterpret_cast<char *>(&pagesPerBlockIn), sizeof(pagesPerBlockIn));

    blocks.clear();
    lbaToPhysical.clear();
    while (!wearMinHeap.empty()) wearMinHeap.pop();
    while (!pendingWrites.empty()) pendingWrites.pop();
    pendingData.clear();
    cache.clear();

    totalBlocks = numBlocks;
    pagesPerBlock = pagesPerBlockIn;
    totalCapacity = numBlocks * pagesPerBlockIn;
    deviceName = "SSD";

    for (int i = 0; i < numBlocks; i++) {
        Block b(i, pagesPerBlockIn);
        int wear = 0;
        in.read(reinterpret_cast<char *>(&wear), sizeof(wear));

        for (int p = 0; p < pagesPerBlockIn; p++) {
            int stateVal = 0, lba = -1;
            in.read(reinterpret_cast<char *>(&stateVal), sizeof(stateVal));
            in.read(reinterpret_cast<char *>(&lba), sizeof(lba));
            string data = readStringBin(in);

            if (stateVal == 1) {
                b.getPageRef(p).writeData(data, lba);
            } else if (stateVal == 2) {
                b.getPageRef(p).writeData(data, lba);
                b.getPageRef(p).invalidate();
            }
            // stateVal == 0 means FREE, which is already the default
        }

        b.setWearCount(wear);
        blocks.push_back(b);
        wearMinHeap.push({wear, i});
    }

    int mapSize = 0;
    in.read(reinterpret_cast<char *>(&mapSize), sizeof(mapSize));
    for (int i = 0; i < mapSize; i++) {
        int lba = 0, loc = 0;
        in.read(reinterpret_cast<char *>(&lba), sizeof(lba));
        in.read(reinterpret_cast<char *>(&loc), sizeof(loc));
        lbaToPhysical[lba] = loc;
    }

    int reads = 0, writes = 0, erases = 0;
    in.read(reinterpret_cast<char *>(&reads), sizeof(reads));
    in.read(reinterpret_cast<char *>(&writes), sizeof(writes));
    in.read(reinterpret_cast<char *>(&erases), sizeof(erases));

    stats.reset();
    for (int i = 0; i < reads; i++) stats.recordRead();
    for (int i = 0; i < writes; i++) stats.recordWrite();
    for (int i = 0; i < erases; i++) stats.recordErase();

    in.close();
    cout << "SSD state loaded from " << filename << endl;
}

void SSD::showStatistics() const {
    stats.display(blocks);
}

int SSD::getTotalBlocks() const {
    return totalBlocks;
}

int SSD::getPagesPerBlock() const {
    return pagesPerBlock;
}
