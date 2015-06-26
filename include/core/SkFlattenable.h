/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFlattenable_DEFINED
#define SkFlattenable_DEFINED

#include "SkRefCnt.h"

class SkReadBuffer;
class SkWriteBuffer;

/*
 *  Flattening is straight-forward:
 *      1. call getFactory() so we have a function-ptr to recreate the subclass
 *      2. call flatten(buffer) to write out enough data for the factory to read
 *
 *  Unflattening is easy for the caller: new_instance = factory(buffer)
 *
 *  The complexity of supporting this is as follows.
 *
 *  If your subclass wants to control unflattening, use this macro in your declaration:
 *      SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS
 *  This will provide a getFactory(), and require that the subclass implements CreateProc.
 *
 *  For older buffers (before the DEEPFLATTENING change, the macros below declare
 *  a thin factory DeepCreateProc. It checks the version of the buffer, and if it is pre-deep,
 *  then it calls through to a (usually protected) constructor, passing the buffer.
 *  If the buffer is newer, then it directly calls the "real" factory: CreateProc.
 */

#define SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP() static void InitializeFlattenables();

#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(flattenable) \
    void flattenable::InitializeFlattenables() {

#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END \
    }

#define SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(flattenable) \
    SkFlattenable::Registrar(#flattenable, flattenable::CreateProc, \
                             flattenable::GetFlattenableType());

#define SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(flattenable)    \
    private:                                                                \
    static SkFlattenable* CreateProc(SkReadBuffer&);                        \
    friend class SkPrivateEffectInitializer;                                \
    public:                                                                 \
    Factory getFactory() const override { return CreateProc; }

/** For SkFlattenable derived objects with a valid type
    This macro should only be used in base class objects in core
  */
#define SK_DEFINE_FLATTENABLE_TYPE(flattenable) \
    static Type GetFlattenableType() { \
        return k##flattenable##_Type; \
    }

/** \class SkFlattenable

 SkFlattenable is the base class for objects that need to be flattened
 into a data stream for either transport or as part of the key to the
 font cache.
 */
class SK_API SkFlattenable : public SkRefCnt {
public:
    enum Type {
        kSkColorFilter_Type,
        kSkDrawLooper_Type,
        kSkImageFilter_Type,
        kSkMaskFilter_Type,
        kSkPathEffect_Type,
        kSkPixelRef_Type,
        kSkRasterizer_Type,
        kSkShader_Type,
        kSkUnused_Type,     // used to be SkUnitMapper
        kSkXfermode_Type,
    };

    typedef SkFlattenable* (*Factory)(SkReadBuffer&);

    SkFlattenable() {}

    /** Implement this to return a factory function pointer that can be called
     to recreate your class given a buffer (previously written to by your
     override of flatten().
     */
    virtual Factory getFactory() const = 0;

    /** Returns the name of the object's class
      */
    const char* getTypeName() const { return FactoryToName(getFactory()); }

    static Factory NameToFactory(const char name[]);
    static const char* FactoryToName(Factory);
    static bool NameToType(const char name[], Type* type);

    static void Register(const char name[], Factory, Type);

    class Registrar {
    public:
        Registrar(const char name[], Factory factory, Type type) {
            SkFlattenable::Register(name, factory, type);
        }
    };

    /**
     *  Override this if your subclass needs to record data that it will need to recreate itself
     *  from its CreateProc (returned by getFactory()).
     */
    virtual void flatten(SkWriteBuffer&) const {}

private:
    static void InitializeFlattenablesIfNeeded();

    friend class SkGraphics;

    typedef SkRefCnt INHERITED;
};

#endif
