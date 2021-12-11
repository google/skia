/*
* Copyright 2021 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkSLDebuggerSlide_DEFINED
#define SkSLDebuggerSlide_DEFINED

#include "src/sksl/tracing/SkVMDebugTrace.h"
#include "src/sksl/tracing/SkVMDebugTracePlayer.h"
#include "tools/viewer/Slide.h"

class SkSLDebuggerSlide : public Slide {
public:
    SkSLDebuggerSlide();

    SkISize getDimensions() const override { return SkISize::MakeEmpty(); }

    void draw(SkCanvas* canvas) override;
    bool animate(double nanos) override;

    void resize(SkScalar winWidth, SkScalar winHeight) override {}
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    bool onMouse(SkScalar x, SkScalar y, skui::InputState state,
                 skui::ModifierKey modifiers) override { return true; }

private:
    void showRootGUI();
    void showLoadTraceGUI();
    void showDebuggerGUI();

    sk_sp<SkSL::SkVMDebugTrace> fTrace;
    SkSL::SkVMDebugTracePlayer fPlayer;

    char fTraceFile[256] = "SkVMDebugTrace.json";
};

#endif
