/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VisualDebugModule_DEFINED
#define VisualDebugModule_DEFINED

#include "VisualModule.h"

#include "SkDebugCanvas.h"
#include "VisualBench.h"
#include "VisualBenchmarkStream.h"

class VisualDebugModule : public VisualModule {
public:
    VisualDebugModule(VisualBench* owner);
    void draw(SkCanvas* canvas) override;
    bool onHandleChar(SkUnichar unichar) override;

private:
    enum State {
        kInit_State,
        kPlay_State,
        kNext_State,
    };

    bool advanceIfNecessary(SkCanvas*);

    State fState;
    SkAutoTUnref<SkDebugCanvas> fDebugCanvas;
    int fIndex;

    // support framework
    VisualBench* fOwner;
    SkAutoTDelete<VisualBenchmarkStream> fBenchmarkStream;

    typedef VisualModule INHERITED;
};

#endif
