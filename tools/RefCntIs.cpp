#include "RefCntIs.h"

bool RefCntIs(const SkRefCntBase& obj, int32_t n) {
    return obj.fRefCnt == n;
}

bool WeakRefCntIs(const SkWeakRefCnt& obj, int32_t n) {
    return obj.fWeakCnt == n;
}
