/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkImageDecoder.h"
#include "SkImageInfo.h"
#include "SkStream.h"
#include "SkString.h"

/*
 *
 * This benchmark is designed to test the performance of subset decoding.
 * Choose subsets to mimic a user zooming in or out on a photo.
 *
 */
class SubsetZoomBench : public Benchmark {
public:

    SubsetZoomBench(const SkString& path,
                    SkColorType colorType,
                    uint32_t subsetWidth,
                    uint32_t subsetHeight,
                    bool useCodec);

protected:
    const char* onGetName() override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(const int n, SkCanvas* canvas) override;

private:
    SkString                      fName;
    SkColorType                   fColorType;
    const uint32_t                fSubsetWidth;
    const uint32_t                fSubsetHeight;
    const bool                    fUseCodec;
    SkAutoTDelete<SkMemoryStream> fStream;
    typedef Benchmark INHERITED;
};
