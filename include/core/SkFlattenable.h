
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFlattenable_DEFINED
#define SkFlattenable_DEFINED

#include "SkRefCnt.h"
#include "SkBitmap.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkReader32.h"
#include "SkTDArray.h"
#include "SkWriter32.h"

class SkFlattenableReadBuffer;
class SkFlattenableWriteBuffer;
class SkString;

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#define SK_DEFINE_FLATTENABLE_REGISTRAR(flattenable) \
    static SkFlattenable::Registrar g##flattenable##Reg(#flattenable, \
                                                       flattenable::CreateProc);
#define SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(flattenable) \
    static SkFlattenable::Registrar g##flattenable##Reg(#flattenable, \
                                                       flattenable::CreateProc);

#define SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(flattenable)
#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

#else

#define SK_DEFINE_FLATTENABLE_REGISTRAR(flattenable)
#define SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(flattenable) \
        SkFlattenable::Registrar(#flattenable, flattenable::CreateProc);

#define SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP() static void InitializeFlattenables();

#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(flattenable) \
    void flattenable::InitializeFlattenables() {

#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END \
    }

#endif

#define SK_DECLARE_UNFLATTENABLE_OBJECT() \
    virtual Factory getFactory() SK_OVERRIDE { return NULL; }; \

#define SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(flattenable) \
    virtual Factory getFactory() SK_OVERRIDE { return CreateProc; }; \
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) { \
        return SkNEW_ARGS(flattenable, (buffer)); \
    }

/** \class SkFlattenable
 
 SkFlattenable is the base class for objects that need to be flattened
 into a data stream for either transport or as part of the key to the
 font cache.
 */
class SK_API SkFlattenable : public SkRefCnt {
public:
    typedef SkFlattenable* (*Factory)(SkFlattenableReadBuffer&);
    
    SkFlattenable() {}
    
    /** Implement this to return a factory function pointer that can be called
     to recreate your class given a buffer (previously written to by your
     override of flatten().
     */
    virtual Factory getFactory() = 0;
    
    static Factory NameToFactory(const char name[]);
    static const char* FactoryToName(Factory);
    static void Register(const char name[], Factory);

    class Registrar {
    public:
        Registrar(const char name[], Factory factory) {
            SkFlattenable::Register(name, factory);
        }
    };

protected:
    SkFlattenable(SkFlattenableReadBuffer&) {}
    /** Override this to write data specific to your subclass into the buffer,
     being sure to call your super-class' version first. This data will later
     be passed to your Factory function, returned by getFactory().
     */
    virtual void flatten(SkFlattenableWriteBuffer&) const;

private:
#if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
    static void InitializeFlattenables();
#endif

    friend class SkGraphics;
    friend class SkFlattenableWriteBuffer;
};

// helpers for matrix and region

class SkMatrix;
extern void SkReadMatrix(SkReader32*, SkMatrix*);
extern void SkWriteMatrix(SkWriter32*, const SkMatrix&);

class SkRegion;
extern void SkReadRegion(SkReader32*, SkRegion*);
extern void SkWriteRegion(SkWriter32*, const SkRegion&);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class SkTypeface;

class SkFlattenableReadBuffer {
public:
    SkFlattenableReadBuffer();
    virtual ~SkFlattenableReadBuffer() {}


    virtual uint8_t readU8() = 0;
    virtual uint16_t readU16() = 0;
    virtual uint32_t readU32() = 0;
    virtual void read(void* dst, size_t size) = 0;
    virtual bool readBool() = 0;
    virtual int32_t readInt() = 0;
    virtual SkScalar readScalar() = 0;
    virtual const void* skip(size_t size) = 0;

    virtual int32_t readS32() { return readInt(); }
    template <typename T> const T& skipT() {
        SkASSERT(SkAlign4(sizeof(T)) == sizeof(T));
        return *(const T*)this->skip(sizeof(T));
    }

    virtual void readMatrix(SkMatrix*) = 0;
    virtual void readPath(SkPath*) = 0;
    virtual void readPoint(SkPoint*) = 0;

    // helper function for classes with const SkPoint members
    SkPoint readPoint() {
        SkPoint point;
        this->readPoint(&point);
        return point;
    }

    void setRefCntArray(SkRefCnt* array[], int count) {
        fRCArray = array;
        fRCCount = count;
    }
    
    void setTypefaceArray(SkTypeface* array[], int count) {
        fTFArray = array;
        fTFCount = count;
    }

    /**
     *  Call this with a pre-loaded array of Factories, in the same order as
     *  were created/written by the writer. SkPicture uses this.
     */
    void setFactoryPlayback(SkFlattenable::Factory array[], int count) {
        fFactoryTDArray = NULL;
        fFactoryArray = array;
        fFactoryCount = count;
    }

    /**
     *  Call this with an initially empty array, so the reader can cache each
     *  factory it sees by name. Used by the pipe code in conjunction with
     *  the writer's kInlineFactoryNames_Flag.
     */
    void setFactoryArray(SkTDArray<SkFlattenable::Factory>* array) {
        fFactoryTDArray = array;
        fFactoryArray = NULL;
        fFactoryCount = 0;
    }
    
    virtual SkTypeface* readTypeface() = 0;
    virtual SkRefCnt* readRefCnt() = 0;
    virtual void* readFunctionPtr() = 0;
    virtual SkFlattenable* readFlattenable() = 0;
    
protected:
    SkRefCnt** fRCArray;
    int        fRCCount;
    
    SkTypeface** fTFArray;
    int        fTFCount;
    
    SkTDArray<SkFlattenable::Factory>* fFactoryTDArray;
    SkFlattenable::Factory* fFactoryArray;
    int                     fFactoryCount;
};

///////////////////////////////////////////////////////////////////////////////

#include "SkPtrRecorder.h"

/**
 *  Subclass of SkTPtrSet specialed to call ref() and unref() when the
 *  base class's incPtr() and decPtr() are called. This makes it a valid owner
 *  of each ptr, which is released when the set is reset or destroyed.
 */
class SkRefCntSet : public SkTPtrSet<SkRefCnt*> {
public:
    virtual ~SkRefCntSet();

protected:
    // overrides
    virtual void incPtr(void*);
    virtual void decPtr(void*);
};

class SkFactorySet : public SkTPtrSet<SkFlattenable::Factory> {};

class SkFlattenableWriteBuffer {
public:
    SkFlattenableWriteBuffer();
    virtual ~SkFlattenableWriteBuffer();

    // deprecated naming convention that will be removed after callers are updated
    virtual bool writeBool(bool value) = 0;
    virtual void writeInt(int32_t value) = 0;
    virtual void write8(int32_t value) = 0;
    virtual void write16(int32_t value) = 0;
    virtual void write32(int32_t value) = 0;
    virtual void writeScalar(SkScalar value) = 0;
    virtual void writeMul4(const void* values, size_t size) = 0;

    virtual void writePad(const void* src, size_t size) = 0;
    virtual void writeString(const char* str, size_t len = (size_t)-1) = 0;
    virtual uint32_t* reserve(size_t size) = 0;
    virtual void flatten(void* dst) = 0;
    virtual uint32_t size() = 0;
    virtual void write(const void* values, size_t size) = 0;
    virtual void writeRect(const SkRect& rect) = 0;
    virtual size_t readFromStream(SkStream*, size_t length) = 0;

    virtual void writeMatrix(const SkMatrix& matrix) = 0;
    virtual void writePath(const SkPath& path) = 0;
    virtual void writePoint(const SkPoint& point) = 0;

    virtual bool writeToStream(SkWStream*) = 0;

    virtual void writeFunctionPtr(void*)= 0;
    virtual void writeFlattenable(SkFlattenable* flattenable)= 0;

    void writeTypeface(SkTypeface*);
    void writeRefCnt(SkRefCnt* obj);

    SkRefCntSet* getTypefaceRecorder() const { return fTFSet; }
    SkRefCntSet* setTypefaceRecorder(SkRefCntSet*);

    SkRefCntSet* getRefCntRecorder() const { return fRCSet; }
    SkRefCntSet* setRefCntRecorder(SkRefCntSet*);

    SkFactorySet* getFactoryRecorder() const { return fFactorySet; }
    SkFactorySet* setFactoryRecorder(SkFactorySet*);

    enum Flags {
        kCrossProcess_Flag       = 0x01,
        /**
         *  Instructs the writer to inline Factory names as there are seen the
         *  first time (after that we store an index). The pipe code uses this.
         */
        kInlineFactoryNames_Flag = 0x02
    };
    Flags getFlags() const { return (Flags)fFlags; }
    void setFlags(Flags flags) { fFlags = flags; }

    bool isCrossProcess() const {
        return SkToBool(fFlags & kCrossProcess_Flag);
    }
    bool inlineFactoryNames() const {
        return SkToBool(fFlags & kInlineFactoryNames_Flag);
    }

    bool persistBitmapPixels() const {
        return (fFlags & kCrossProcess_Flag) != 0;
    }

    bool persistTypeface() const { return (fFlags & kCrossProcess_Flag) != 0; }

protected:

    // A helper function so that each subclass does not have to be a friend of
    // SkFlattenable.
    void flattenObject(SkFlattenable* obj, SkFlattenableWriteBuffer& buffer) {
        obj->flatten(buffer);
    }

    uint32_t        fFlags;
    SkRefCntSet*    fTFSet;
    SkRefCntSet*    fRCSet;
    SkFactorySet*   fFactorySet;
};

#endif

