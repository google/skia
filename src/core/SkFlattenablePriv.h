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
 */

#define SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(flattenable)     \
    SkFlattenable::Register(#flattenable,                      \
                            flattenable::CreateProc,           \
                            flattenable::GetFlattenableType());

#define SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(flattenable) \
    private:                                                             \
        static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);           \
        friend class SkFlattenable::PrivateInitializer;                  \
    public:                                                              \
        Factory getFactory() const override { return flattenable::CreateProc; }

#endif
