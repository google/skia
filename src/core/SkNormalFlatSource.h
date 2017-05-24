/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNormalFlatSource_DEFINED
#define SkNormalFlatSource_DEFINED

#include "SkNormalSource.h"

class SK_API SkNormalFlatSourceImpl : public SkNormalSource {
public:
    SkNormalFlatSourceImpl(){}

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const SkShader::AsFPArgs&) const override;
#endif

    SkNormalSource::Provider* asProvider(const SkShader::ContextRec& rec,
                                         SkArenaAlloc* alloc) const override;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkNormalFlatSourceImpl)

protected:
    void flatten(SkWriteBuffer& buf) const override;

private:
    class Provider : public SkNormalSource::Provider {
    public:
        Provider();

        ~Provider() override;

        void fillScanLine(int x, int y, SkPoint3 output[], int count) const override;

    private:
        typedef SkNormalSource::Provider INHERITED;
    };

    friend class SkNormalSource;

    typedef SkNormalSource INHERITED;
};

#endif
