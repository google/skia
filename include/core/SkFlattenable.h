
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
        SkFlattenable::Registrar(#flattenable, flattenable::CreateProc, \
                                 flattenable::GetFlattenableType());

#define SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP() static void InitializeFlattenables();

#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(flattenable) \
    void flattenable::InitializeFlattenables() {

#define SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END \
    }

#define SK_DECLARE_UNFLATTENABLE_OBJECT() \
    virtual Factory getFactory() const SK_OVERRIDE { return NULL; }

#define SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(flattenable) \
    virtual Factory getFactory() const SK_OVERRIDE { return CreateProc; } \
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) { \
        return SkNEW_ARGS(flattenable, (buffer)); \
    }

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
        kSkUnitMapper_Type,
        kSkXfermode_Type,
    };

    SK_DECLARE_INST_COUNT(SkFlattenable)

    typedef SkFlattenable* (*Factory)(SkFlattenableReadBuffer&);

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

protected:
    SkFlattenable(SkFlattenableReadBuffer&) {}
    /** Override this to write data specific to your subclass into the buffer,
     being sure to call your super-class' version first. This data will later
     be passed to your Factory function, returned by getFactory().
     */
    virtual void flatten(SkFlattenableWriteBuffer&) const;

private:
    static void InitializeFlattenablesIfNeeded();

    friend class SkGraphics;
    friend class SkFlattenableWriteBuffer;

    typedef SkRefCnt INHERITED;
};

#endif
