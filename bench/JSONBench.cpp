/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/utils/SkJSON.h"

#if defined(SK_BUILD_FOR_ANDROID)
static constexpr const char* kBenchFile = "/data/local/tmp/bench.json";
#else
static constexpr const char* kBenchFile = "/tmp/bench.json";
#endif

class JsonBench : public Benchmark {
public:

protected:
    const char* onGetName() override { return "json_skjson"; }

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onPerCanvasPreDraw(SkCanvas*) override {
        fData = SkData::MakeFromFileName(kBenchFile);
        if (!fData) {
            SkDebugf("!! Could not open bench file: %s\n", kBenchFile);
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        fData = nullptr;
    }

    void onDraw(int loops, SkCanvas*) override {
        if (!fData) return;

        for (int i = 0; i < loops; i++) {
            skjson::DOM dom(static_cast<const char*>(fData->data()), fData->size());
            if (dom.root().is<skjson::NullValue>()) {
                SkDebugf("!! Parsing failed.\n");
                return;
            }
        }
    }

private:
    sk_sp<SkData> fData;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new JsonBench; )

#if (0)

#include "rapidjson/document.h"

class RapidJsonBench : public Benchmark {
public:

protected:
    const char* onGetName() override { return "json_rapidjson"; }

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onPerCanvasPreDraw(SkCanvas*) override {
        if (auto stream = SkStream::MakeFromFile(kBenchFile)) {
            SkASSERT(stream->hasLength());
            fCStringData = SkData::MakeUninitialized(stream->getLength() + 1);
            auto* data8 = reinterpret_cast<uint8_t*>(fCStringData->writable_data());
            SkAssertResult(stream->read(data8, stream->getLength()) == stream->getLength());
            data8[stream->getLength()] = '\0';

        } else {
            SkDebugf("!! Could not open bench file: %s\n", kBenchFile);
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        fCStringData = nullptr;
    }

    void onDraw(int loops, SkCanvas*) override {
        if (!fCStringData) return;

        for (int i = 0; i < loops; i++) {
            rapidjson::Document doc;
            doc.Parse(static_cast<const char*>(fCStringData->data()));
            if (doc.HasParseError()) {
                SkDebugf("!! Parsing failed.\n");
                return;
            }
        }
    }

private:
    sk_sp<SkData> fCStringData;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new RapidJsonBench; )

#endif

#if (0)

#include "pjson.h"

class PJsonBench : public Benchmark {
public:

protected:
    const char* onGetName() override { return "json_pjson"; }

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onPerCanvasPreDraw(SkCanvas*) override {
        if (auto stream = SkStream::MakeFromFile(kBenchFile)) {
            SkASSERT(stream->hasLength());
            fCStringData = SkData::MakeUninitialized(stream->getLength() + 1);
            auto* data8 = reinterpret_cast<uint8_t*>(fCStringData->writable_data());
            SkAssertResult(stream->read(data8, stream->getLength()) == stream->getLength());
            data8[stream->getLength()] = '\0';

        } else {
            SkDebugf("!! Could not open bench file: %s\n", kBenchFile);
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        fCStringData = nullptr;
    }

    void onDraw(int loops, SkCanvas*) override {
        if (!fCStringData) return;

        for (int i = 0; i < loops; i++) {
            // Copy needed for in-place operation.
            auto data = SkData::MakeWithCopy(fCStringData->data(), fCStringData->size());
            pjson::document doc;
            if (!doc.deserialize_in_place(static_cast<char*>(data->writable_data()))) {
                SkDebugf("!! Parsing failed.\n");
                return;
            }
        }
    }

private:
    sk_sp<SkData> fCStringData;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new PJsonBench; )

#endif
