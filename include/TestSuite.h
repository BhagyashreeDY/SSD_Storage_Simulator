#ifndef TEST_SUITE_H
#define TEST_SUITE_H

class TestSuite {
public:
    static void runAllTests();

private:
    static void testEmptyRead();
    static void testInvalidAddressRead();
    static void testDuplicateWrite();
    static void testUpdateNonexistentAddress();
    static void testFullSSDWrite();
};

#endif
