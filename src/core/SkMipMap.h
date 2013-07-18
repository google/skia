#ifndef SkMipMap_DEFINED
#define SkMipMap_DEFINED

#include "SkRefCnt.h"
#include "SkScalar.h"

class SkBitmap;

class SkMipMap : public SkRefCnt {
public:
    static SkMipMap* Build(const SkBitmap& src);

    struct Level {
        void*       fPixels;
        uint32_t    fRowBytes;
        uint32_t    fWidth, fHeight;
    };
    
    bool extractLevel(SkScalar scale, Level*) const;

private:
    Level*  fLevels;
    int     fCount;

    // we take ownership of levels, and will free it with sk_free()
    SkMipMap(Level* levels, int count) : fLevels(levels), fCount(count) {
        SkASSERT(levels);
        SkASSERT(count > 0);
    }

    virtual ~SkMipMap() {
        sk_free(fLevels);
    }

    static Level* AllocLevels(int levelCount, size_t pixelSize);
};

#endif
