/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDebugMarkerOp_DEFINED
#define GrDebugMarkerOp_DEFINED

#include "GrOp.h"
#include "GrRenderTargetProxy.h"

class GrOpFlushState;

class GrDebugMarkerOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrContext*,
                                      GrRenderTargetProxy*,
                                      const SkString&);

    const char* name() const override { return "DebugMarker"; }

    SkString dumpInfo() const override {
        SkString string;
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    friend class GrOpMemoryPool; // for ctor

    GrDebugMarkerOp(GrRenderTargetProxy* proxy, const SkString& str)
            : INHERITED(ClassID())
            , fStr(str) {
        // Make this cover the whole screen so it can't be reordered around
        this->makeFullScreen(proxy);
    }

    bool onCombineIfPossible(GrOp* that, const GrCaps& caps) override { return false; }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState* state) override;

    SkString fStr;

    typedef GrOp INHERITED;
};

#endif
