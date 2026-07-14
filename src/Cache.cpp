#include "Cache.h"

Cache::Cache(int cap) {
    capacity = cap;
}

bool Cache::get(int lba, string &outData) {
    auto it = cacheMap.find(lba);
    if (it == cacheMap.end()) return false;
    outData = it->second;
    return true;
}

void Cache::put(int lba, const string &data) {
    if (cacheMap.find(lba) == cacheMap.end()) {
        // clear out stale front entries before checking capacity
        while (!insertOrder.empty() && cacheMap.find(insertOrder.front()) == cacheMap.end()) {
            insertOrder.pop();
        }
        if ((int)cacheMap.size() >= capacity && !insertOrder.empty()) {
            int oldest = insertOrder.front();
            insertOrder.pop();
            cacheMap.erase(oldest);
        }
        insertOrder.push(lba);
    }
    cacheMap[lba] = data;
}

void Cache::invalidate(int lba) {
    cacheMap.erase(lba);
}

void Cache::clear() {
    cacheMap.clear();
    while (!insertOrder.empty()) insertOrder.pop();
}
