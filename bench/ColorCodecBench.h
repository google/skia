/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ColorCodecBench_DEFINED
#define ColorCodecBench_DEFINED

#include "Benchmark.h"
#include "SkData.h"
#include "SkImageInfo.h"

#if defined(SK_TEST_QCMS)
#include "qcms.h"
#endif

class ColorCodecBench : public Benchmark {
public:
    ColorCodecBench(const char* name, sk_sp<SkData> encoded);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int n, SkCanvas* canvas) override;
    void onDelayedSetup() override;

private:
    void decodeAndXform();
    void xformOnly();
#if !defined(GOOGLE3)
    void decodeAndXformQCMS();
    void xformOnlyQCMS();
#endif

    SkString                                             fName;
    sk_sp<SkData>                                        fEncoded;
    SkImageInfo                                          fInfo;
    SkAutoMalloc                                         fDst;
    SkAutoMalloc                                         fSrc;
    sk_sp<SkColorSpace>                                  fDstSpace;
#if defined(SK_TEST_QCMS)
    SkAutoTCallVProc<qcms_profile, qcms_profile_release> fDstSpaceQCMS;
#endif
    sk_sp<SkData>                                        fSrcData;

    typedef Benchmark INHERITED;
};
#endif // ColorCodecBench_DEFINED
