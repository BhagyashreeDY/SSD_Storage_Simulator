#include "Page.h"

Page::Page() {
    pageId = -1;
    state = PageState::FREE;
    data = "";
    mappedLBA = -1;
}

Page::Page(int id) {
    pageId = id;
    state = PageState::FREE;
    data = "";
    mappedLBA = -1;
}

void Page::writeData(const string &d, int lba) {
    data = d;
    mappedLBA = lba;
    state = PageState::VALID;
}

void Page::invalidate() {
    state = PageState::INVALID;
    mappedLBA = -1;
}

void Page::reset() {
    state = PageState::FREE;
    data = "";
    mappedLBA = -1;
}

bool Page::isFree() const {
    return state == PageState::FREE;
}

bool Page::isValidPage() const {
    return state == PageState::VALID;
}

bool Page::isInvalidPage() const {
    return state == PageState::INVALID;
}

string Page::getData() const {
    return data;
}

int Page::getMappedLBA() const {
    return mappedLBA;
}

int Page::getPageId() const {
    return pageId;
}
