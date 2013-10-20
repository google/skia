
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

#define SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(flattenable) \
        SkFlattenable::Registrar(#flattenable, flattenable::CreateProc);

#define SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP() static void InitializeFlattenables();

#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(flattenable) \
    void flattenable::InitializeFlattenables() {

#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END \
    }

#define SK_DECLARE_UNFLATTENABLE_OBJECT() \
    virtual Factory getFactory() SK_OVERRIDE { return NULL; }

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
    static void InitializeFlattenables();

    friend class SkGraphics;
    friend class SkFlattenableWriteBuffer;

    typedef SkRefCnt INHERITED;
};

#endif
