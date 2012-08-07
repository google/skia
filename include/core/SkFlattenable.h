
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFlattenable_DEFINED
#define SkFlattenable_DEFINED

#include "SkRefCnt.h"

class SkFlattenableReadBuffer;
class SkFlattenableWriteBuffer;

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
    virtual Factory getFactory() SK_OVERRIDE { return CreateProc; } \
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
    SK_DECLARE_INST_COUNT(SkFlattenable)

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

    typedef SkRefCnt INHERITED;
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

/**
 * Similar to SkFactorySet, but only allows Factorys that have registered names.
 * Also has a function to return the next added Factory's name.
 */
class SkNamedFactorySet : public SkRefCnt {
public:
    SkNamedFactorySet();

    /**
     * Find the specified Factory in the set. If it is not already in the set,
     * and has registered its name, add it to the set, and return its index.
     * If the Factory has no registered name, return 0.
     */
    uint32_t find(SkFlattenable::Factory);

    /**
     * If new Factorys have been added to the set, return the name of the first
     * Factory added after the Factory name returned by the last call to this
     * function.
     */
    const char* getNextAddedFactoryName();
private:
    int                    fNextAddedFactory;
    SkFactorySet           fFactorySet;
    SkTDArray<const char*> fNames;
};

#endif
