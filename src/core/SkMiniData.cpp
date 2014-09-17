#include "SkMiniData.h"

namespace {

// SkMiniData::fRep either stores a LongData* or is punned into a ShortData.
// We use the low bits to distinguish the two: all pointers from malloc are at
// least 8-byte aligned, leaving those low bits clear when it's a LongData*.

static bool is_long(uint64_t rep) {
    // Even on 32-bit machines, we require the bottom 3 bits from malloc'd pointers are clear.
    // If any of those bottom 3 bits are set, it's from a ShortData's len.  And if no bits are
    // set anywhere, it's an empty SkMiniData, which also follows the ShortData path.
    return rep && SkIsAlign8(rep);
}

// Can be used for any length, but we always use it for >=8.
struct LongData {
    size_t len;
    uint8_t data[8];  // There are actually len >= 8 bytes here.

    static uint64_t Create(const void* data, size_t len) {
        SkASSERT(len > 7);
        LongData* s = (LongData*)sk_malloc_throw(sizeof(size_t) + len);
        s->len = len;
        memcpy(s->data, data, len);

        uint64_t rep = reinterpret_cast<uint64_t>(s);
        SkASSERT(is_long(rep));
        return rep;
    }
};

// At most 7 bytes fit, but never mallocs.
struct ShortData {
    // Order matters here. len must align with the least signficant bits of a pointer.
#ifdef SK_CPU_LENDIAN
    uint8_t len;
    uint8_t data[7];
#else  // Warning!  Only the little-endian path has been tested.
    uint8_t data[7];
    uint8_t len;
#endif

    static uint64_t Create(const void* data, size_t len) {
        SkASSERT(len <= 7);
#ifdef SK_CPU_LENDIAN
        ShortData s = { (uint8_t)len, {0, 0, 0, 0, 0, 0, 0} };
#else  // Warning!  Only the little-endian path has been tested.
        ShortData s = { {0, 0, 0, 0, 0, 0, 0}, (uint8_t)len };
#endif
        memcpy(s.data, data, len);
        return *reinterpret_cast<uint64_t*>(&s);
    }
};

}  // namespace

SkMiniData::SkMiniData(const void* data, size_t len)
    : fRep(len <= 7 ? ShortData::Create(data, len)
                    :  LongData::Create(data, len)) {}

SkMiniData::SkMiniData(const SkMiniData& s)
    : fRep(s.len() <= 7 ? ShortData::Create(s.data(), s.len())
                        :  LongData::Create(s.data(), s.len())) {}

SkMiniData::~SkMiniData() {
    if (is_long(fRep)) {
        sk_free(reinterpret_cast<void*>(fRep));
    }
}

const void* SkMiniData::data() const {
    return is_long(fRep) ? reinterpret_cast<const  LongData*>( fRep)->data
                         : reinterpret_cast<const ShortData*>(&fRep)->data;
}

size_t SkMiniData::len() const {
    return is_long(fRep) ? reinterpret_cast<const  LongData*>( fRep)->len
                         : reinterpret_cast<const ShortData*>(&fRep)->len;
}
