/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MSKPSlide_DEFINED
#define MSKPSlide_DEFINED

#include "tools/MSKPPlayer.h"
#include "tools/viewer/Slide.h"

class SkStreamSeekable;

class MSKPSlide : public Slide {
public:
    MSKPSlide(const SkString& name, const SkString& path);
    MSKPSlide(const SkString& name, std::unique_ptr<SkStreamSeekable>);

    SkISize getDimensions() const override;

    void draw(SkCanvas* canvas) override;
    bool animate(double nanos) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    void gpuTeardown() override;

private:
    // Call if layers need to be redrawn because we've looped playback or UI interaction.
    void redrawLayers();

    std::unique_ptr<SkStreamSeekable> fStream;
    std::unique_ptr<MSKPPlayer>       fPlayer;

    int    fFrame         = 0;
    int    fFPS           = 15;
    bool   fPaused        = false;
    double fLastFrameTime = -1;

    bool fShowFrameBounds = false;

    // Default to transparent black, which is correct for Android MSKPS.
    float fBackgroundColor[4] = {0, 0, 0, 0};

    std::vector<int>              fAllLayerIDs;
    std::vector<std::vector<int>> fFrameLayerIDs;
    std::vector<SkString>         fLayerIDStrings;
    int                           fDrawLayerID = -1;  // -1 means just draw the root layer
    bool                          fListAllLayers = true;

    using INHERITED = Slide;
};

#endif
