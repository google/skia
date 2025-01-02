/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include "bench/Benchmark.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkWebpDecoder.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkAutoMalloc.h"
#include "tools/Resources.h"

namespace {

class WebpBlendBench final : public Benchmark {
public:
    const char* onGetName() override {
        return "webp_blend_bench";
    }

    bool isSuitableFor(Backend backend) override {
        return Backend::kNonRendering == backend;
    }

    void onDelayedSetup() override {
        auto data = GetResourceAsData("images/blendBG.webp");
        SkASSERT(data);

        SkCodec::Result result;
        fCodec = SkWebpDecoder::Decode(data, &result);
        SkASSERT(result == SkCodec::kSuccess);

        fStorage.reset(fCodec->getInfo().computeMinByteSize());
    }

    void onDraw(int n, SkCanvas* canvas) override {
        const SkImageInfo info = fCodec->getInfo();

        SkCodec::Options options;
        options.fFrameIndex = 1; // frame known to blend with prev

        for (int i = 0; i < n; ++i) {
            fCodec->getPixels(info, fStorage.get(), info.minRowBytes(), &options);
        }
    }

private:
    std::unique_ptr<SkCodec> fCodec;
    SkAutoMalloc             fStorage;
};

}  // namespace

DEF_BENCH( return new WebpBlendBench(); )
