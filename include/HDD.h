#ifndef HDD_H
#define HDD_H

#include <vector>
#include "StorageDevice.h"
using namespace std;

// HDD is intentionally simple - direct sector addressing, no wear
// leveling, no garbage collection, no address translation. its only
// real purpose in this project is to prove that write()/read()/erase()
// behave completely differently for SSD vs HDD even though both are
// called through the same StorageDevice* interface.
class HDD : public StorageDevice {
private:
    vector<string> sectors;
    vector<bool> occupied;

public:
    HDD(int numSectors);

    bool write(int logicalAddr, const string &data) override;
    string read(int logicalAddr) override;
    bool erase(int blockId) override;
    void display() const override;
};

#endif
