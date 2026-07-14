#include "Controller.h"
#include "HDD.h"
#include "SSDException.h"
#include "TestSuite.h"
#include <iostream>
using namespace std;

Controller::Controller() {
    device = nullptr;
    ssd = nullptr;
}

Controller::~Controller() {
    if (device != nullptr) {
        delete device;
    }
}

bool Controller::readInt(int &out) const {
    cin >> out;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "That's not a valid number, operation cancelled." << endl;
        return false;
    }
    return true;
}

bool Controller::deviceExists() const {
    if (device == nullptr) {
        cout << "No device created yet. Use option 1 or 12 first." << endl;
        return false;
    }
    return true;
}

void Controller::createSSD() {
    int numBlocks, pagesPerBlock;
    cout << "Enter number of blocks: ";
    if (!readInt(numBlocks)) return;
    cout << "Enter pages per block: ";
    if (!readInt(pagesPerBlock)) return;

    if (numBlocks <= 0 || pagesPerBlock <= 0) {
        cout << "Blocks and pages per block must be positive numbers." << endl;
        return;
    }

    if (device != nullptr) {
        delete device;
        device = nullptr;
    }

    ssd = new SSD(numBlocks, pagesPerBlock);
    device = ssd;
    cout << "SSD created with " << numBlocks << " blocks x " << pagesPerBlock << " pages." << endl;
}

void Controller::createHDD() {
    int numSectors;
    cout << "Enter number of sectors: ";
    if (!readInt(numSectors)) return;

    if (numSectors <= 0) {
        cout << "Number of sectors must be positive." << endl;
        return;
    }

    if (device != nullptr) {
        delete device;
        device = nullptr;
    }

    ssd = nullptr;  // SSD-only operations are disabled while an HDD is active
    device = new HDD(numSectors);
    cout << "HDD created with " << numSectors << " sectors." << endl;
    cout << "(HDD is here to show that write/read/erase behave differently under the same interface)" << endl;
}

void Controller::handleWrite() {
    if (!deviceExists()) return;

    int addr;
    cout << "Enter logical address: ";
    if (!readInt(addr)) return;

    string data;
    cout << "Enter data: ";
    cin >> data;

    try {
        device->write(addr, data);
        cout << "Write successful." << endl;
    } catch (SSDException &e) {
        cout << "Write failed: " << e.what() << endl;
    }
}

void Controller::handleRead() {
    if (!deviceExists()) return;

    int addr;
    cout << "Enter logical address: ";
    if (!readInt(addr)) return;

    try {
        string result = device->read(addr);
        cout << "Data at LBA " << addr << " : " << result << endl;
    } catch (SSDException &e) {
        cout << "Read failed: " << e.what() << endl;
    }
}

void Controller::handleUpdate() {
    if (!deviceExists()) return;

    if (ssd == nullptr) {
        cout << "Update is only supported for SSD in this project." << endl;
        return;
    }

    int addr;
    cout << "Enter logical address to update: ";
    if (!readInt(addr)) return;

    string data;
    cout << "Enter new data: ";
    cin >> data;

    try {
        ssd->updateData(addr, data);
        cout << "Update successful." << endl;
    } catch (SSDException &e) {
        cout << "Update failed: " << e.what() << endl;
    }
}

void Controller::handleDelete() {
    if (!deviceExists()) return;

    if (ssd == nullptr) {
        cout << "Delete is only supported for SSD in this project." << endl;
        return;
    }

    int addr;
    cout << "Enter logical address to delete: ";
    if (!readInt(addr)) return;

    if (ssd->deleteData(addr)) {
        cout << "Delete successful." << endl;
    } else {
        cout << "That address was not mapped, nothing to delete." << endl;
    }
}

void Controller::handleDisplay() {
    if (!deviceExists()) return;
    device->display();
}

void Controller::handleGarbageCollection() {
    if (!deviceExists()) return;

    if (ssd == nullptr) {
        cout << "Garbage collection only applies to SSD." << endl;
        return;
    }

    ssd->runGarbageCollection();
    cout << "Garbage collection pass complete." << endl;
}

void Controller::handleStatistics() {
    if (!deviceExists()) return;

    if (ssd == nullptr) {
        cout << "Statistics view in this project only applies to SSD." << endl;
        return;
    }

    ssd->showStatistics();
}

void Controller::handleSave() {
    if (ssd == nullptr) {
        cout << "Only SSD state can be saved in this project." << endl;
        return;
    }

    string filename;
    cout << "Enter filename to save (e.g. data/ssd_state.bin): ";
    cin >> filename;
    ssd->saveState(filename);
}

void Controller::handleLoad() {
    string filename;
    cout << "Enter filename to load (e.g. data/ssd_state.bin): ";
    cin >> filename;

    if (device != nullptr) {
        delete device;
        device = nullptr;
    }

    ssd = new SSD();
    ssd->loadState(filename);
    device = ssd;
}

void Controller::handleQueueWrite() {
    if (!deviceExists()) return;

    if (ssd == nullptr) {
        cout << "Pending write queue is only demonstrated for SSD." << endl;
        return;
    }

    int addr;
    cout << "Enter logical address: ";
    if (!readInt(addr)) return;

    string data;
    cout << "Enter data: ";
    cin >> data;

    ssd->queueWrite(addr, data);
    cout << "Write queued. Use option 14 to flush pending writes." << endl;
}

void Controller::handleProcessPending() {
    if (!deviceExists()) return;

    if (ssd == nullptr) {
        cout << "Pending write queue is only demonstrated for SSD." << endl;
        return;
    }

    try {
        ssd->processPendingWrites();
        cout << "All pending writes processed." << endl;
    } catch (SSDException &e) {
        cout << "Processing pending writes stopped early: " << e.what() << endl;
    }
}

void Controller::showMenu() const {
    cout << "\n================ SSD Storage Simulator ================" << endl;
    cout << " 1. Create SSD" << endl;
    cout << " 2. Write Data" << endl;
    cout << " 3. Read Data" << endl;
    cout << " 4. Update Data" << endl;
    cout << " 5. Delete Data" << endl;
    cout << " 6. Display Device" << endl;
    cout << " 7. Run Garbage Collection" << endl;
    cout << " 8. Show Statistics" << endl;
    cout << " 9. Save State" << endl;
    cout << "10. Load State" << endl;
    cout << "11. Exit" << endl;
    cout << "12. Create HDD (bonus - shows polymorphism against SSD)" << endl;
    cout << "13. Queue a Write (pending write demo)" << endl;
    cout << "14. Process Pending Writes" << endl;
    cout << "15. Run Test Suite (boundary case tests)" << endl;
    cout << "=========================================================" << endl;
    cout << "Choice: ";
}

void Controller::run() {
    bool running = true;
    while (running) {
        showMenu();
        int choice;
        cin >> choice;

        if (cin.fail()) {
            if (cin.eof()) {
                cout << "\nNo more input. Exiting." << endl;
                break;
            }
            // user typed something that isn't a number - clear the
            // error flag and throw away the bad token so we don't
            // spin on the same invalid input forever
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Please enter a valid number." << endl;
            continue;
        }

        switch (choice) {
            case 1: createSSD(); break;
            case 2: handleWrite(); break;
            case 3: handleRead(); break;
            case 4: handleUpdate(); break;
            case 5: handleDelete(); break;
            case 6: handleDisplay(); break;
            case 7: handleGarbageCollection(); break;
            case 8: handleStatistics(); break;
            case 9: handleSave(); break;
            case 10: handleLoad(); break;
            case 11: running = false; cout << "Exiting." << endl; break;
            case 12: createHDD(); break;
            case 13: handleQueueWrite(); break;
            case 14: handleProcessPending(); break;
            case 15: TestSuite::runAllTests(); break;
            default: cout << "Invalid choice, try again." << endl; break;
        }
    }
}
