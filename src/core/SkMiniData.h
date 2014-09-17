#ifndef SkMiniData_DEFINED
#define SkMiniData_DEFINED

// A class that can store any immutable byte string,
// but optimized to store <=7 bytes.

#include "SkTypes.h"

class SkMiniData {
public:
    SkMiniData(const void*, size_t);
    SkMiniData(const SkMiniData&);
    ~SkMiniData();

    const void* data() const;
    size_t len() const;

private:
    SkMiniData& operator=(const SkMiniData&);

    const uint64_t fRep;
};

#endif//SkMiniData_DEFINED
