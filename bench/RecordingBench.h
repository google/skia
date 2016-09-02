/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RecordingBench_DEFINED
#define RecordingBench_DEFINED

#include "Benchmark.h"
#include "SkPicture.h"
#include "SkLiteDL.h"

class RecordingBench : public Benchmark {
public:
    RecordingBench(const char* name, const SkPicture*, bool useBBH, bool lite);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend) override;
    void onDraw(int loops, SkCanvas*) override;
    SkIPoint onGetSize() override;

private:
    sk_sp<const SkPicture> fSrc;
    SkString fName;
    sk_sp<SkLiteDL> fDL;
    bool fUseBBH;

    typedef Benchmark INHERITED;
};

#endif//RecordingBench_DEFINED
