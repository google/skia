/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNormalBevelSource_DEFINED
#define SkNormalBevelSource_DEFINED

#include "SkNormalSource.h"

class SK_API SkNormalBevelSourceImpl : public SkNormalSource {
public:
    SkNormalBevelSourceImpl(BevelType type, SkScalar width, SkScalar height)
        : fType(type)
        , fWidth(width)
        , fHeight(height) {}

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const SkShader::AsFPArgs&) const override;
#endif

    SkNormalSource::Provider* asProvider(const SkShader::ContextRec& rec,
                                         SkArenaAlloc*) const override;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkNormalBevelSourceImpl)

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

    SkNormalSource::BevelType fType;
    SkScalar fWidth;
    SkScalar fHeight;

    friend class SkNormalSource;

    typedef SkNormalSource INHERITED;
};


#endif
