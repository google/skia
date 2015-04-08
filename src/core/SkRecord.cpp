#include "SkRecord.h"

SkRecord::~SkRecord() {
    Destroyer destroyer;
    for (unsigned i = 0; i < this->count(); i++) {
        this->mutate<void>(i, destroyer);
    }
}

void SkRecord::grow() {
    SkASSERT(fCount == fReserved);
    fReserved = SkTMax<unsigned>(kFirstReserveCount, fReserved*2);
    fRecords.realloc(fReserved);
    fTypes.realloc(fReserved);
}

size_t SkRecord::bytesUsed() const {
    return fAlloc.approxBytesAllocated() +
           fReserved * (sizeof(Record) + sizeof(Type8)) +
           sizeof(SkRecord);
}
