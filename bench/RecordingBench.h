/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RecordingBench_DEFINED
#define RecordingBench_DEFINED

#include "bench/Benchmark.h"
#include "include/core/SkPicture.h"

class PictureCentricBench : public Benchmark {
public:
    PictureCentricBench(const char* name, const SkPicture*);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend) override;
    SkIPoint onGetSize() override;

protected:
    sk_sp<const SkPicture> fSrc;
    SkString fName;

    typedef Benchmark INHERITED;
};

class RecordingBench : public PictureCentricBench {
public:
    RecordingBench(const char* name, const SkPicture*, bool useBBH);

protected:
    void onDraw(int loops, SkCanvas*) override;

private:
    bool fUseBBH;

    typedef PictureCentricBench INHERITED;
};

class DeserializePictureBench : public Benchmark {
public:
    DeserializePictureBench(const char* name, sk_sp<SkData> encodedPicture);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend) override;
    SkIPoint onGetSize() override;
    void onDraw(int loops, SkCanvas*) override;

private:
    SkString      fName;
    sk_sp<SkData> fEncodedPicture;

    typedef Benchmark INHERITED;
};

#endif//RecordingBench_DEFINED
