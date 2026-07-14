#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>
#include <queue>
#include <string>
using namespace std;

// small fixed-size cache for recently read logical addresses.
// eviction order is tracked with a queue (oldest inserted gets
// evicted first). stale entries in the queue (already invalidated)
// are just skipped when encountered - cheaper than removing from
// the middle of a queue.
class Cache {
private:
    int capacity;
    unordered_map<int, string> cacheMap;
    queue<int> insertOrder;

public:
    Cache(int cap = 5);

    bool get(int lba, string &outData);
    void put(int lba, const string &data);
    void invalidate(int lba);
    void clear();
};

#endif
