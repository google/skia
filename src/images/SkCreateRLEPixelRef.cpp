#include "SkChunkAlloc.h"
#include "SkPackBits.h"
#include "SkBitmap.h"
#include "SkPixelRef.h"

class RLEPixelRef : public SkPixelRef {
public:
    RLEPixelRef(SkBitmap::RLEPixels* rlep, SkColorTable* ctable);
    virtual ~RLEPixelRef();
    
protected:
    // overrides from SkPixelRef
    virtual void* onLockPixels(SkColorTable**);
    virtual void onUnlockPixels();
    
private:
    SkBitmap::RLEPixels* fRLEPixels;
    SkColorTable*        fCTable;
};

RLEPixelRef::RLEPixelRef(SkBitmap::RLEPixels* rlep, SkColorTable* ctable)
        : SkPixelRef(NULL) {
    fRLEPixels = rlep;  // we now own this ptr
    fCTable = ctable;
    ctable->safeRef();
}

RLEPixelRef::~RLEPixelRef() {
    SkDELETE(fRLEPixels);
    fCTable->safeUnref();
}

void* RLEPixelRef::onLockPixels(SkColorTable** ct) {
    *ct = fCTable;
    return fRLEPixels;
}

void RLEPixelRef::onUnlockPixels() {
    // nothing to do
}

/////////////////////////////////////////////////////////////////////////////

class ChunkRLEPixels : public SkBitmap::RLEPixels {
public:
    ChunkRLEPixels(int width, int height, size_t chunkSize)
        : SkBitmap::RLEPixels(width, height), fStorage(chunkSize) {
    }
    
    SkChunkAlloc fStorage;
};

SkPixelRef* SkCreateRLEPixelRef(const SkBitmap& src);
SkPixelRef* SkCreateRLEPixelRef(const SkBitmap& src) {
    
    if (SkBitmap::kIndex8_Config != src.config() &&
            SkBitmap::kA8_Config != src.config()) {
        return NULL;
    }
    
    size_t maxPacked = SkPackBits::ComputeMaxSize8(src.width());

    // estimate the rle size based on the original size
    size_t size = src.getSize() >> 3;
    if (size < maxPacked) {
        size = maxPacked;
    }

    ChunkRLEPixels* rlePixels = SkNEW_ARGS(ChunkRLEPixels,
                                           (src.width(), src.height(), size));

    uint8_t* dstRow = NULL;
    size_t free = 0;
    size_t totalPacked = 0;

    for (int y = 0; y < src.height(); y++) {
        const uint8_t* srcRow = src.getAddr8(0, y);
        
        if (free < maxPacked) {
            dstRow = (uint8_t*)rlePixels->fStorage.allocThrow(size);
            free = size;
        }
        size_t packedSize = SkPackBits::Pack8(srcRow, src.width(), dstRow);
        SkASSERT(packedSize <= free);
        rlePixels->setPackedAtY(y, dstRow);
        
        dstRow += packedSize;
        free -= packedSize;
        
        totalPacked += packedSize;
    }
    
//#ifdef SK_DEBUG
#if 0
    // test
    uint8_t* buffer = new uint8_t[src.width()];
    for (int y = 0; y < src.height(); y++) {
        const uint8_t* srcRow = src.getAddr8(0, y);
        SkPackBits::Unpack8(buffer, 0, src.width(), rlePixels->packedAtY(y));
        int n = memcmp(buffer, srcRow, src.width());
        if (n) {
            SkDebugf("----- memcmp returned %d on line %d\n", n, y);
        }
        SkASSERT(n == 0);
    }
    delete[] buffer;

    size_t totalAlloc = src.height() * sizeof(uint8_t*) + totalPacked;
    
    SkDebugf("--- RLE: orig [%d %d] %d, rle %d %d savings %g\n",
             src.width(), src.height(), src.getSize(),
             src.height() * sizeof(uint8_t*), totalPacked,
             (float)totalAlloc / src.getSize());

#endif

    // transfer ownership of rlePixels to our pixelref
    return SkNEW_ARGS(RLEPixelRef, (rlePixels, src.getColorTable()));
}
        
