/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeque.h"
#include "SkLayerRasterizer.h"
#include "SkPaint.h"
#include "SkRasterizer.h"
#include "Test.h"

class SkReadBuffer;

// Dummy class to place on a paint just to ensure the paint's destructor
// is called.
// ONLY to be used by LayerRasterizer_destructor, since other tests may
// be run in a separate thread, and this class is not threadsafe.
class DummyRasterizer : public SkRasterizer {
public:
    DummyRasterizer()
        : INHERITED()
    {
        // Not threadsafe. Only used in one thread.
        gCount++;
    }

    ~DummyRasterizer() {
        // Not threadsafe. Only used in one thread.
        gCount--;
    }

    static int GetCount() { return gCount; }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(DummyRasterizer);

private:
    static int gCount;

    typedef SkRasterizer INHERITED;
};

int DummyRasterizer::gCount;

SkFlattenable* DummyRasterizer::CreateProc(SkReadBuffer&) {
    return SkNEW(DummyRasterizer);
}

// Check to make sure that the SkPaint in the layer has its destructor called.
DEF_TEST(LayerRasterizer_destructor, reporter) {
    {
        SkPaint paint;
        paint.setRasterizer(SkNEW(DummyRasterizer))->unref();
        REPORTER_ASSERT(reporter, DummyRasterizer::GetCount() == 1);

        SkLayerRasterizer::Builder builder;
        builder.addLayer(paint);
    }
    REPORTER_ASSERT(reporter, DummyRasterizer::GetCount() == 0);
}

class LayerRasterizerTester {
public:
    static int CountLayers(const SkLayerRasterizer& layerRasterizer) {
        return layerRasterizer.fLayers->count();
    }

    static const SkDeque& GetLayers(const SkLayerRasterizer& layerRasterizer) {
        return *layerRasterizer.fLayers;
    }
};

// MUST stay in sync with definition of SkLayerRasterizer_Rec in SkLayerRasterizer.cpp.
struct SkLayerRasterizer_Rec {
    SkPaint     fPaint;
    SkVector    fOffset;
};

static bool equals(const SkLayerRasterizer_Rec& rec1, const SkLayerRasterizer_Rec& rec2) {
    return rec1.fPaint == rec2.fPaint && rec1.fOffset == rec2.fOffset;
}

DEF_TEST(LayerRasterizer_copy, reporter) {
    SkLayerRasterizer::Builder builder;
    REPORTER_ASSERT(reporter, NULL == builder.snapshotRasterizer());
    SkPaint paint;
    // Create a bunch of paints with different flags.
    for (uint32_t flags = 0x01; flags < SkPaint::kAllFlags; flags <<= 1) {
        paint.setFlags(flags);
        builder.addLayer(paint, static_cast<SkScalar>(flags), static_cast<SkScalar>(flags));
    }

    // Create a layer rasterizer with all the existing layers.
    SkAutoTUnref<SkLayerRasterizer> firstCopy(builder.snapshotRasterizer());

    // Add one more layer.
    paint.setFlags(SkPaint::kAllFlags);
    builder.addLayer(paint);

    SkAutoTUnref<SkLayerRasterizer> oneLarger(builder.snapshotRasterizer());
    SkAutoTUnref<SkLayerRasterizer> detached(builder.detachRasterizer());

    // Check the counts for consistency.
    const int largerCount = LayerRasterizerTester::CountLayers(*oneLarger.get());
    const int smallerCount = LayerRasterizerTester::CountLayers(*firstCopy.get());
    REPORTER_ASSERT(reporter, largerCount == LayerRasterizerTester::CountLayers(*detached.get()));
    REPORTER_ASSERT(reporter, smallerCount == largerCount - 1);

    const SkLayerRasterizer_Rec* recFirstCopy = NULL;
    const SkLayerRasterizer_Rec* recOneLarger = NULL;
    const SkLayerRasterizer_Rec* recDetached = NULL;

    const SkDeque& layersFirstCopy = LayerRasterizerTester::GetLayers(*firstCopy.get());
    const SkDeque& layersOneLarger = LayerRasterizerTester::GetLayers(*oneLarger.get());
    const SkDeque& layersDetached = LayerRasterizerTester::GetLayers(*detached.get());

    // Ensure that our version of SkLayerRasterizer_Rec is the same as the one in
    // SkLayerRasterizer.cpp - or at least the same size. If the order were switched, we
    // would fail the test elsewhere.
    REPORTER_ASSERT(reporter, layersFirstCopy.elemSize() == sizeof(SkLayerRasterizer_Rec));
    REPORTER_ASSERT(reporter, layersOneLarger.elemSize() == sizeof(SkLayerRasterizer_Rec));
    REPORTER_ASSERT(reporter, layersDetached.elemSize() == sizeof(SkLayerRasterizer_Rec));

    SkDeque::F2BIter iterFirstCopy(layersFirstCopy);
    SkDeque::F2BIter iterOneLarger(layersOneLarger);
    SkDeque::F2BIter iterDetached(layersDetached);

    for (int i = 0; i < largerCount; ++i) {
        recFirstCopy = static_cast<const SkLayerRasterizer_Rec*>(iterFirstCopy.next());
        recOneLarger = static_cast<const SkLayerRasterizer_Rec*>(iterOneLarger.next());
        recDetached  = static_cast<const SkLayerRasterizer_Rec*>(iterDetached.next());

        REPORTER_ASSERT(reporter, equals(*recOneLarger, *recDetached));
        if (smallerCount == i) {
            REPORTER_ASSERT(reporter, recFirstCopy == NULL);
        } else {
            REPORTER_ASSERT(reporter, equals(*recFirstCopy, *recOneLarger));
        }
    }
}

DEF_TEST(LayerRasterizer_detachEmpty, reporter) {
    SkLayerRasterizer::Builder builder;
    REPORTER_ASSERT(reporter, NULL == builder.detachRasterizer());
}
