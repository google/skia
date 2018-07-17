/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFlattenable_DEFINED
#define SkFlattenable_DEFINED

#include "SkRefCnt.h"

class SkData;
class SkReadBuffer;
class SkWriteBuffer;

struct SkSerialProcs;
struct SkDeserialProcs;

/** \class SkFlattenable

 SkFlattenable is the base class for objects that need to be flattened
 into a data stream for either transport or as part of the key to the
 font cache.
 */
class SK_API SkFlattenable : public SkRefCnt {
public:
    enum Type {
        kSkColorFilter_Type,
        kSkDrawable_Type,
        kSkDrawLooper_Type,
        kSkImageFilter_Type,
        kSkMaskFilter_Type,
        kSkPathEffect_Type,
        kSkPixelRef_Type,
        kSkUnused_Type4,    // used to be SkRasterizer
        kSkShaderBase_Type,
        kSkUnused_Type,     // used to be SkUnitMapper
        kSkUnused_Type2,
        kSkNormalSource_Type,
    };

    typedef sk_sp<SkFlattenable> (*Factory)(SkReadBuffer&);

    SkFlattenable() {}

    /** Implement this to return a factory function pointer that can be called
     to recreate your class given a buffer (previously written to by your
     override of flatten().
     */
    virtual Factory getFactory() const = 0;

    /**
     *  Returns the name of the object's class.
     *
     *  Subclasses should override this function if they intend to provide
     *  support for flattening without using the global registry.
     *
     *  If the flattenable is registered, there is no need to override.
     */
    virtual const char* getTypeName() const { return FactoryToName(getFactory()); }

    static Factory NameToFactory(const char name[]);
    static const char* FactoryToName(Factory);
    static bool NameToType(const char name[], Type* type);

    static void Register(const char name[], Factory, Type);

    /**
     *  Override this if your subclass needs to record data that it will need to recreate itself
     *  from its CreateProc (returned by getFactory()).
     *
     *  DEPRECATED public : will move to protected ... use serialize() instead
     */
    virtual void flatten(SkWriteBuffer&) const {}

    virtual Type getFlattenableType() const = 0;

    //
    // public ways to serialize / deserialize
    //
    sk_sp<SkData> serialize(const SkSerialProcs* = nullptr) const;
    size_t serialize(void* memory, size_t memory_size,
                     const SkSerialProcs* = nullptr) const;
    static sk_sp<SkFlattenable> Deserialize(Type, const void* data, size_t length,
                                            const SkDeserialProcs* procs = nullptr);

protected:
    class PrivateInitializer {
    public:
        static void InitCore();
        static void InitEffects();
        static void InitImageFilters();
    };

private:
    static void InitializeFlattenablesIfNeeded();
    static void Finalize();

    friend class SkGraphics;

    typedef SkRefCnt INHERITED;
};

#endif
