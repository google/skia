#include "SkFlattenable.h"
#include "SkTypeface.h"

#include "SkMatrix.h"
#include "SkRegion.h"

void SkReadMatrix(SkReader32* reader, SkMatrix* matrix) {
    size_t size = matrix->unflatten(reader->peek());
    SkASSERT(SkAlign4(size) == size);
    (void)reader->skip(size);
}

void SkWriteMatrix(SkWriter32* writer, const SkMatrix& matrix) {
    size_t size = matrix.flatten(NULL);
    SkASSERT(SkAlign4(size) == size);
    matrix.flatten(writer->reserve(size));
}

void SkReadRegion(SkReader32* reader, SkRegion* rgn) {
    size_t size = rgn->unflatten(reader->peek());
    SkASSERT(SkAlign4(size) == size);
    (void)reader->skip(size);
}

void SkWriteRegion(SkWriter32* writer, const SkRegion& rgn) {
    size_t size = rgn.flatten(NULL);
    SkASSERT(SkAlign4(size) == size);
    rgn.flatten(writer->reserve(size));
}

///////////////////////////////////////////////////////////////////////////////

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

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
}

SkFlattenableReadBuffer::SkFlattenableReadBuffer(const void* data) :
        INHERITED(data, 1024 * 1024) {
    fRCArray = NULL;
    fRCCount = 0;

    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
}

SkFlattenableReadBuffer::SkFlattenableReadBuffer(const void* data, size_t size)
        : INHERITED(data, size) {
    fRCArray = NULL;
    fRCCount = 0;

    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
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
        int32_t index = this->readU32();
        if (0 == index) {
            return NULL; // writer failed to give us the flattenable
        }
        index = -index; // we stored the negative of the index
        index -= 1;     // we stored the index-base-1
        SkASSERT(index < (unsigned)fFactoryCount);
        factory = fFactoryArray[index];
    } else if (fFactoryTDArray) {
        const int32_t* peek = (const int32_t*)this->peek();
        if (*peek <= 0) {
            int32_t index = this->readU32();
            if (0 == index) {
                return NULL; // writer failed to give us the flattenable
            }
            index = -index; // we stored the negative of the index
            index -= 1;     // we stored the index-base-1
            factory = (*fFactoryTDArray)[index];
        } else {
            const char* name = this->readString();
            factory = SkFlattenable::NameToFactory(name);
            if (factory) {
                SkASSERT(fFactoryTDArray->find(factory) < 0);
                *fFactoryTDArray->append() = factory;
            } else {
//                SkDebugf("can't find factory for [%s]\n", name);
            }
            // if we didn't find a factory, that's our failure, not the writer's,
            // so we fall through, so we can skip the sizeRecorded data.
        }
    } else {
        factory = (SkFlattenable::Factory)readFunctionPtr();
        if (NULL == factory) {
            return NULL; // writer failed to give us the flattenable
        }
    }

    // if we get here, factory may still be null, but if that is the case, the
    // failure was ours, not the writer.
    SkFlattenable* obj = NULL;
    uint32_t sizeRecorded = this->readU32();
    if (factory) {
        uint32_t offset = this->offset();
        obj = (*factory)(*this);
        // check that we read the amount we expected
        uint32_t sizeRead = this->offset() - offset;
        if (sizeRecorded != sizeRead) {
            // we could try to fix up the offset...
            sk_throw();
        }
    } else {
        // we must skip the remaining data
        this->skip(sizeRecorded);
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
    fRCSet = NULL;
    fTFSet = NULL;
    fFactorySet = NULL;
}

SkFlattenableWriteBuffer::~SkFlattenableWriteBuffer() {
    SkSafeUnref(fRCSet);
    SkSafeUnref(fTFSet);
    SkSafeUnref(fFactorySet);
}

SkRefCntSet* SkFlattenableWriteBuffer::setRefCntRecorder(SkRefCntSet* rec) {
    SkRefCnt_SafeAssign(fRCSet, rec);
    return rec;
}

SkRefCntSet* SkFlattenableWriteBuffer::setTypefaceRecorder(SkRefCntSet* rec) {
    SkRefCnt_SafeAssign(fTFSet, rec);
    return rec;
}

SkFactorySet* SkFlattenableWriteBuffer::setFactoryRecorder(SkFactorySet* rec) {
    SkRefCnt_SafeAssign(fFactorySet, rec);
    return rec;
}

void SkFlattenableWriteBuffer::writeTypeface(SkTypeface* obj) {
    if (NULL == obj || NULL == fTFSet) {
        this->write32(0);
    } else {
        this->write32(fTFSet->add(obj));
    }
}

void SkFlattenableWriteBuffer::writeRefCnt(SkRefCnt* obj) {
    if (NULL == obj || NULL == fRCSet) {
        this->write32(0);
    } else {
        this->write32(fRCSet->add(obj));
    }
}

void SkFlattenableWriteBuffer::writeFlattenable(SkFlattenable* flattenable) {
    /*
     *  If we have a factoryset, then the first 32bits tell us...
     *       0: failure to write the flattenable
     *      <0: we store the negative of the (1-based) index
     *      >0: the length of the name
     *  If we don't have a factoryset, then the first "ptr" is either the
     *  factory, or null for failure.
     *
     *  The distinction is important, since 0-index is 32bits (always), but a
     *  0-functionptr might be 32 or 64 bits.
     */

    SkFlattenable::Factory factory = NULL;
    if (flattenable) {
        factory = flattenable->getFactory();
    }
    if (NULL == factory) {
        if (fFactorySet) {
            this->write32(0);
        } else {
            this->writeFunctionPtr(NULL);
        }
        return;
    }

    /*
     *  We can write 1 of 3 versions of the flattenable:
     *  1.  function-ptr : this is the fastest for the reader, but assumes that
     *      the writer and reader are in the same process.
     *  2.  index into fFactorySet : This is assumes the writer will later
     *      resolve the function-ptrs into strings for its reader. SkPicture
     *      does exactly this, by writing a table of names (matching the indices)
     *      up front in its serialized form.
     *  3.  names : Reuse fFactorySet to store indices, but only after we've
     *      written the name the first time. SkGPipe uses this technique, as it
     *      doesn't require the reader to be told to know the table of names
     *      up front.
     */
    if (fFactorySet) {
        if (this->inlineFactoryNames()) {
            int index = fFactorySet->find(factory);
            if (index) {
                // we write the negative of the index, to distinguish it from
                // the length of a string
                this->write32(-index);
            } else {
                const char* name = SkFlattenable::FactoryToName(factory);
                if (NULL == name) {
                    this->write32(0);
                    return;
                }
                this->writeString(name);
                index = fFactorySet->add(factory);
            }
        } else {
            // we write the negative of the index, to distinguish it from
            // the length of a string
            this->write32(-fFactorySet->add(factory));
        }
    } else {
        this->writeFunctionPtr((void*)factory);
    }

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

void SkFlattenableWriteBuffer::writeFunctionPtr(void* proc) {
    *(void**)this->reserve(sizeof(void*)) = proc;
}

///////////////////////////////////////////////////////////////////////////////

SkRefCntSet::~SkRefCntSet() {
    // call this now, while our decPtr() is sill in scope
    this->reset();
}

void SkRefCntSet::incPtr(void* ptr) {
    ((SkRefCnt*)ptr)->ref();
}

void SkRefCntSet::decPtr(void* ptr) {
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

