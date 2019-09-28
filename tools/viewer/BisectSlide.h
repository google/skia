/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BisectSlide_DEFINED
#define BisectSlide_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "tools/viewer/Slide.h"

#include <stack>

/**
 * This is a simple utility designed to extract the paths from an SKP file and then isolate a single
 * one of them via bisect. Use the 'x' and 'X' keys to guide a binary search:
 *
 *   'x': Throw out half the paths.
 *   'X': Toggle which half gets tossed and which half is kept.
 *   'Z': Back up one level.
 *   'D': Dump the path.
 */
class BisectSlide : public Slide, public SkCanvas {
public:
    static sk_sp<BisectSlide> Create(const char filepath[]);

    // Slide overrides.
    SkISize getDimensions() const override { return fDrawBounds.size(); }
    bool onChar(SkUnichar c) override;
    void draw(SkCanvas* canvas) override;

private:
    BisectSlide(const char filepath[]);

    // SkCanvas override called only during creation.
    void onDrawPath(const SkPath& path, const SkPaint& paint) override;

    struct FoundPath {
        SkPath fPath;
        SkPaint fPaint;
        SkMatrix fViewMatrix;
    };

    SkString fFilePath;
    SkIRect fDrawBounds = SkIRect::MakeEmpty();
    SkTArray<FoundPath> fFoundPaths;
    SkTArray<FoundPath> fTossedPaths;
    SkTArray<char> fTrail;
    std::stack<std::pair<SkTArray<FoundPath>, SkTArray<FoundPath>>> fPathHistory;
};

#endif
