#ifndef PAGE_H
#define PAGE_H

#include <string>
using namespace std;

// A page can only be in one of these states at a time.
// FREE    -> never written, or reset after the whole block was erased
// VALID   -> holds live data that is currently mapped by an LBA
// INVALID -> used to hold data, but has since been overwritten/deleted.
//            cannot be reused for writing until the entire block is erased
//            (this mirrors how real NAND flash pages behave)
enum class PageState { FREE, VALID, INVALID };

class Page {
private:
    int pageId;
    PageState state;
    string data;
    int mappedLBA;

public:
    Page();
    Page(int id);

    void writeData(const string &d, int lba);
    void invalidate();
    void reset();

    bool isFree() const;
    bool isValidPage() const;
    bool isInvalidPage() const;

    string getData() const;
    int getMappedLBA() const;
    int getPageId() const;
};

#endif
