/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFlattenablePriv_DEFINED
#define SkFlattenablePriv_DEFINED

#include "SkFlattenable.h"

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
    SkFlattenable::Register(#flattenable, flattenable::CreateProc, \
                            flattenable::GetFlattenableType());

#define SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(flattenable)    \
    private:                                                                \
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);                        \
    friend class SkFlattenable::PrivateInitializer;                         \
    public:                                                                 \
    Factory getFactory() const override { return CreateProc; }

/** For SkFlattenable derived objects with a valid type
    This macro should only be used in base class objects in core
  */
#define SK_DEFINE_FLATTENABLE_TYPE(flattenable) \
    static Type GetFlattenableType() {          \
        return k##flattenable##_Type;           \
    }                                           \
    Type getFlattenableType() const override {  \
        return k##flattenable##_Type;           \
    }                                           \
    static sk_sp<flattenable> Deserialize(const void* data, size_t size,                \
                                          const SkDeserialProcs* procs = nullptr) {     \
        return sk_sp<flattenable>(static_cast<flattenable*>(                            \
                                  SkFlattenable::Deserialize(                           \
                                  k##flattenable##_Type, data, size, procs).release()));\
    }


#endif
