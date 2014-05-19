#ifndef RecordTestUtils_DEFINED
#define RecordTestUtils_DEFINED

#include "SkRecord.h"
#include "SkRecords.h"

// If the command we're reading is a U, set ptr to it, otherwise set it to NULL.
template <typename U>
struct ReadAs {
    ReadAs() : ptr(NULL), type(SkRecords::Type(~0)) {}

    const U* ptr;
    SkRecords::Type type;

    void operator()(const U& r) { ptr = &r; type = U::kType; }

    template <typename T>
    void operator()(const T&) { type = U::kType; }
};

// Assert that the ith command in record is of type T, and return it.
template <typename T>
static const T* assert_type(skiatest::Reporter* r, const SkRecord& record, unsigned index) {
    ReadAs<T> reader;
    record.visit<void>(index, reader);
    REPORTER_ASSERT(r, T::kType == reader.type);
    REPORTER_ASSERT(r, NULL != reader.ptr);
    return reader.ptr;
}

#endif//RecordTestUtils_DEFINED
