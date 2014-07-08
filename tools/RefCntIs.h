#ifndef RefCntIs_DEFINED
#define RefCntIs_DEFINED

// Allows tests to assert a particular value for a ref count,
// without letting Skia library code look at that value.

#include "SkRefCnt.h"
#include "SkWeakRefCnt.h"

bool RefCntIs(const SkRefCntBase&, int32_t);
bool WeakRefCntIs(const SkWeakRefCnt&, int32_t);

#endif//RefCntIs_DEFINED
