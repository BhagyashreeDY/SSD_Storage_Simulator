#ifndef SSD_EXCEPTION_H
#define SSD_EXCEPTION_H

#include <exception>
#include <string>
using namespace std;

class SSDException : public exception {
protected:
    string message;

public:
    SSDException(const string &msg) {
        message = msg;
    }

    const char *what() const noexcept override {
        return message.c_str();
    }
};

class InvalidAddressException : public SSDException {
public:
    InvalidAddressException(int addr)
        : SSDException("Invalid logical address: " + to_string(addr)) {}
};

class SSDFullException : public SSDException {
public:
    SSDFullException()
        : SSDException("SSD is full - no free pages available even after garbage collection") {}
};

class EmptyReadException : public SSDException {
public:
    EmptyReadException()
        : SSDException("Read attempted on an empty/uninitialized device") {}
};

#endif
