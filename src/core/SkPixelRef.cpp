#include "SkPixelRef.h"
#include "SkFlattenable.h"
#include "SkThread.h"

static SkMutex  gPixelRefMutex;

extern int32_t SkNextPixelRefGenerationID() {
    static int32_t  gPixelRefGenerationID;
    // do a loop in case our global wraps around, as we never want to
    // return a 0
    int32_t genID;
    do {
        genID = sk_atomic_inc(&gPixelRefGenerationID) + 1;
    } while (0 == genID);
    return genID;
}


SkPixelRef::SkPixelRef(SkMutex* mutex) {
    if (NULL == mutex) {
        mutex = &gPixelRefMutex;
    }
    fMutex = mutex;
    fPixels = NULL;
    fColorTable = NULL; // we do not track ownership of this
    fLockCount = 0;
    fGenerationID = 0;  // signal to rebuild
    fIsImmutable = false;
}

SkPixelRef::SkPixelRef(SkFlattenableReadBuffer& buffer, SkMutex* mutex) {
    if (NULL == mutex) {
        mutex = &gPixelRefMutex;
    }
    fMutex = mutex;
    fPixels = NULL;
    fColorTable = NULL; // we do not track ownership of this
    fLockCount = 0;
    fGenerationID = 0;  // signal to rebuild
    fIsImmutable = buffer.readBool();
}

void SkPixelRef::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.writeBool(fIsImmutable);
}

void SkPixelRef::lockPixels() {
    SkAutoMutexAcquire  ac(*fMutex);

    if (1 == ++fLockCount) {
        fPixels = this->onLockPixels(&fColorTable);
    }
}

void SkPixelRef::unlockPixels() {
    SkAutoMutexAcquire  ac(*fMutex);

    SkASSERT(fLockCount > 0);
    if (0 == --fLockCount) {
        this->onUnlockPixels();
        fPixels = NULL;
        fColorTable = NULL;
    }
}

uint32_t SkPixelRef::getGenerationID() const {
    if (0 == fGenerationID) {
        fGenerationID = SkNextPixelRefGenerationID();
    }
    return fGenerationID;
}

void SkPixelRef::notifyPixelsChanged() {
#ifdef SK_DEBUG
    if (fIsImmutable) {
        SkDebugf("========== notifyPixelsChanged called on immutable pixelref");
    }
#endif
    // this signals us to recompute this next time around
    fGenerationID = 0;
}

void SkPixelRef::setImmutable() {
    fIsImmutable = true;
}

bool SkPixelRef::readPixels(SkBitmap* dst, const SkIRect* subset) {
    return this->onReadPixels(dst, subset);
}

bool SkPixelRef::onReadPixels(SkBitmap* dst, const SkIRect* subset) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

#define MAX_PAIR_COUNT  16

struct Pair {
    const char*          fName;
    SkPixelRef::Factory  fFactory;
};

static int gCount;
static Pair gPairs[MAX_PAIR_COUNT];

void SkPixelRef::Register(const char name[], Factory factory) {
    SkASSERT(name);
    SkASSERT(factory);

    static bool gOnce;
    if (!gOnce) {
        gCount = 0;
        gOnce = true;
    }

    SkASSERT(gCount < MAX_PAIR_COUNT);

    gPairs[gCount].fName = name;
    gPairs[gCount].fFactory = factory;
    gCount += 1;
}

SkPixelRef::Factory SkPixelRef::NameToFactory(const char name[]) {
    const Pair* pairs = gPairs;
    for (int i = gCount - 1; i >= 0; --i) {
        if (strcmp(pairs[i].fName, name) == 0) {
            return pairs[i].fFactory;
        }
    }
    return NULL;
}

const char* SkPixelRef::FactoryToName(Factory fact) {
    const Pair* pairs = gPairs;
    for (int i = gCount - 1; i >= 0; --i) {
        if (pairs[i].fFactory == fact) {
            return pairs[i].fName;
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef ANDROID
void SkPixelRef::globalRef(void* data) {
    this->ref();
}

void SkPixelRef::globalUnref() {
    this->unref();
}
#endif
