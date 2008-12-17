#include "SkFlattenable.h"
#include "SkTypeface.h"

void SkFlattenable::flatten(SkFlattenableWriteBuffer&)
{
    /*  we don't write anything at the moment, but this allows our subclasses
        to not know that, since we want them to always call INHERITED::flatten()
        in their code.
    */
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkFlattenableReadBuffer::SkFlattenableReadBuffer() {
    fRCArray = NULL;
    fRCCount = 0;
    
    fTFArray = NULL;
    fTFCount = 0;
    
    fFactoryArray = NULL;
    fFactoryCount = 0;
}

SkFlattenableReadBuffer::SkFlattenableReadBuffer(const void* data) :
        INHERITED(data, 1024 * 1024) {
    fRCArray = NULL;
    fRCCount = 0;
    
    fTFArray = NULL;
    fTFCount = 0;
    
    fFactoryArray = NULL;
    fFactoryCount = 0;
}

SkFlattenableReadBuffer::SkFlattenableReadBuffer(const void* data, size_t size)
        : INHERITED(data, size) {
    fRCArray = NULL;
    fRCCount = 0;
    
    fTFArray = NULL;
    fTFCount = 0;
    
    fFactoryArray = NULL;
    fFactoryCount = 0;
}

SkTypeface* SkFlattenableReadBuffer::readTypeface() {
    uint32_t index = this->readU32();
    if (0 == index || index > (unsigned)fTFCount) {
        if (index) {
            SkDebugf("====== typeface index %d\n", index);
        }
        return NULL;
    } else {
        SkASSERT(fTFArray);
        return fTFArray[index - 1];
    }
}

SkRefCnt* SkFlattenableReadBuffer::readRefCnt() {
    uint32_t index = this->readU32();
    if (0 == index || index > (unsigned)fRCCount) {
        return NULL;
    } else {
        SkASSERT(fRCArray);
        return fRCArray[index - 1];
    }
}

SkFlattenable* SkFlattenableReadBuffer::readFlattenable() {
    SkFlattenable::Factory factory = NULL;
    
    if (fFactoryCount > 0) {
        uint32_t index = this->readU32();
        if (index > 0) {
            index -= 1;
            SkASSERT(index < (unsigned)fFactoryCount);
            factory = fFactoryArray[index];
            // if we recorded an index, but failed to get a factory, we need
            // to skip the flattened data in the buffer
            if (NULL == factory) {
                uint32_t size = this->readU32();
                this->skip(size);
                // fall through and return NULL for the object
            }
        }
    } else {
        factory = (SkFlattenable::Factory)readFunctionPtr();
    }

    SkFlattenable* obj = NULL;
    if (factory) {
        uint32_t sizeRecorded = this->readU32();
        uint32_t offset = this->offset();
        obj = (*factory)(*this);
        // check that we read the amount we expected
        uint32_t sizeRead = this->offset() - offset;
        if (sizeRecorded != sizeRead) {
            // we could try to fix up the offset...
            sk_throw();
        }
    }
    return obj;
}

void* SkFlattenableReadBuffer::readFunctionPtr() {
    void* proc;
    this->read(&proc, sizeof(proc));
    return proc;
}

///////////////////////////////////////////////////////////////////////////////

SkFlattenableWriteBuffer::SkFlattenableWriteBuffer(size_t minSize) :
        INHERITED(minSize) {
    fFlags = (Flags)0;
    fRCRecorder = NULL;
    fTFRecorder = NULL;
    fFactoryRecorder = NULL;
}

SkFlattenableWriteBuffer::~SkFlattenableWriteBuffer() {
    fRCRecorder->safeUnref();
    fTFRecorder->safeUnref();
    fFactoryRecorder->safeUnref();
}

SkRefCntRecorder* SkFlattenableWriteBuffer::setRefCntRecorder(
                                                    SkRefCntRecorder* rec) {
    SkRefCnt_SafeAssign(fRCRecorder, rec);
    return rec;
}

SkRefCntRecorder* SkFlattenableWriteBuffer::setTypefaceRecorder(
                                                    SkRefCntRecorder* rec) {
    SkRefCnt_SafeAssign(fTFRecorder, rec);
    return rec;
}

SkFactoryRecorder* SkFlattenableWriteBuffer::setFactoryRecorder(
                                                    SkFactoryRecorder* rec) {
    SkRefCnt_SafeAssign(fFactoryRecorder, rec);
    return rec;
}

void SkFlattenableWriteBuffer::writeTypeface(SkTypeface* obj) {
    if (NULL == obj || NULL == fTFRecorder) {
        this->write32(0);
    } else {
        this->write32(fTFRecorder->record(obj));
    }
}

void SkFlattenableWriteBuffer::writeRefCnt(SkRefCnt* obj) {
    if (NULL == obj || NULL == fRCRecorder) {
        this->write32(0);
    } else {
        this->write32(fRCRecorder->record(obj));
    }
}

void SkFlattenableWriteBuffer::writeFlattenable(SkFlattenable* flattenable) {
    SkFlattenable::Factory factory = NULL;
    if (flattenable) {
        factory = flattenable->getFactory();
    }

    if (fFactoryRecorder) {
        this->write32(fFactoryRecorder->record(factory));
    } else {
        this->writeFunctionPtr((void*)factory);
    }
    
    if (factory) {
        // make room for the size of the flatttened object
        (void)this->reserve(sizeof(uint32_t));
        // record the current size, so we can subtract after the object writes.
        uint32_t offset = this->size();
        // now flatten the object
        flattenable->flatten(*this);
        uint32_t objSize = this->size() - offset;
        // record the obj's size
        *this->peek32(offset - sizeof(uint32_t)) = objSize;
    }
}

void SkFlattenableWriteBuffer::writeFunctionPtr(void* proc) {
    *(void**)this->reserve(sizeof(void*)) = proc;
}

///////////////////////////////////////////////////////////////////////////////

SkRefCntRecorder::~SkRefCntRecorder() {
    // call this now, while our decPtr() is sill in scope
    this->reset();
}

void SkRefCntRecorder::incPtr(void* ptr) {
    ((SkRefCnt*)ptr)->ref();
}

void SkRefCntRecorder::decPtr(void* ptr) {
    ((SkRefCnt*)ptr)->unref();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define MAX_PAIR_COUNT  64

struct Pair {
    const char*             fName;
    SkFlattenable::Factory  fFactory;
};

static int gCount;
static Pair gPairs[MAX_PAIR_COUNT];

void SkFlattenable::Register(const char name[], Factory factory) {
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

SkFlattenable::Factory SkFlattenable::NameToFactory(const char name[]) {
    const Pair* pairs = gPairs;
    for (int i = gCount - 1; i >= 0; --i) {
        if (strcmp(pairs[i].fName, name) == 0) {
            return pairs[i].fFactory;
        }
    }
    return NULL;
}

const char* SkFlattenable::FactoryToName(Factory fact) {
    const Pair* pairs = gPairs;
    for (int i = gCount - 1; i >= 0; --i) {
        if (pairs[i].fFactory == fact) {
            return pairs[i].fName;
        }
    }
    return NULL;
}

bool SkFlattenable::toDumpString(SkString* str) const {
    return false;
}

