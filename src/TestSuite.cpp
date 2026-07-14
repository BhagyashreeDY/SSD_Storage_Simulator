#include "TestSuite.h"
#include "SSD.h"
#include "SSDException.h"
#include <iostream>
using namespace std;

void TestSuite::testEmptyRead() {
    cout << "[Test] Empty read on a brand new SSD ... ";
    SSD ssd(2, 2);
    try {
        ssd.read(0);
        cout << "FAIL (expected an exception, none was thrown)" << endl;
    } catch (InvalidAddressException &e) {
        cout << "PASS (" << e.what() << ")" << endl;
    }
}

void TestSuite::testInvalidAddressRead() {
    cout << "[Test] Read from an address that was never written ... ";
    SSD ssd(2, 2);
    ssd.write(0, "sample");
    try {
        ssd.read(999);
        cout << "FAIL (expected an exception, none was thrown)" << endl;
    } catch (InvalidAddressException &e) {
        cout << "PASS (" << e.what() << ")" << endl;
    }
}

void TestSuite::testDuplicateWrite() {
    cout << "[Test] Duplicate write to the same logical address ... ";
    SSD ssd(2, 2);
    ssd.write(1, "hello");
    ssd.write(1, "world");
    string result = ssd.read(1);
    if (result == "world") {
        cout << "PASS (latest value correctly returned: " << result << ")" << endl;
    } else {
        cout << "FAIL (expected 'world', got '" << result << "')" << endl;
    }
}

void TestSuite::testUpdateNonexistentAddress() {
    cout << "[Test] Update an address that was never written ... ";
    SSD ssd(2, 2);
    try {
        ssd.updateData(50, "should not work");
        cout << "FAIL (expected an exception, none was thrown)" << endl;
    } catch (InvalidAddressException &e) {
        cout << "PASS (" << e.what() << ")" << endl;
    }
}

void TestSuite::testFullSSDWrite() {
    cout << "[Test] Write to a completely full SSD (no invalid pages to reclaim) ... ";
    SSD ssd(1, 2);  // 1 block x 2 pages = 2 pages total, no room for GC to help
    ssd.write(0, "a");
    ssd.write(1, "b");
    try {
        ssd.write(2, "c");
        cout << "FAIL (expected SSDFullException, none was thrown)" << endl;
    } catch (SSDFullException &e) {
        cout << "PASS (" << e.what() << ")" << endl;
    }
}

void TestSuite::runAllTests() {
    cout << "\n================ Running Test Suite ================" << endl;
    testEmptyRead();
    testInvalidAddressRead();
    testDuplicateWrite();
    testUpdateNonexistentAddress();
    testFullSSDWrite();
    cout << "======================================================" << endl;
}
