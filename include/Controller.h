#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "StorageDevice.h"
#include "SSD.h"

class Controller {
private:
    StorageDevice *device;  // common interface - used for write/read/erase/display
    SSD *ssd;                // extra handle for SSD-only operations (GC, save/load, update, delete)

    void createSSD();
    void createHDD();
    void handleWrite();
    void handleRead();
    void handleUpdate();
    void handleDelete();
    void handleDisplay();
    void handleGarbageCollection();
    void handleStatistics();
    void handleSave();
    void handleLoad();
    void handleQueueWrite();
    void handleProcessPending();

    void showMenu() const;
    bool deviceExists() const;
    bool readInt(int &out) const;

public:
    Controller();
    ~Controller();
    void run();
};

#endif
