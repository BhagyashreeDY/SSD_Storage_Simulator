#ifndef STORAGE_DEVICE_H
#define STORAGE_DEVICE_H

#include <string>
using namespace std;

// Abstract base class. SSD and HDD implement read/write/erase/display
// completely differently under the hood, so this class only defines
// the interface every storage device must expose.
class StorageDevice {
protected:
    string deviceName;
    int totalCapacity;

public:
    StorageDevice(string name, int capacity) {
        deviceName = name;
        totalCapacity = capacity;
    }

    virtual bool write(int logicalAddr, const string &data) = 0;
    virtual string read(int logicalAddr) = 0;
    virtual bool erase(int blockId) = 0;
    virtual void display() const = 0;

    string getDeviceName() const { return deviceName; }
    int getTotalCapacity() const { return totalCapacity; }

    // virtual destructor is required here - without it, deleting a
    // derived object through a StorageDevice* would not call the
    // derived class destructor and could leak memory
    virtual ~StorageDevice() {}
};

#endif
