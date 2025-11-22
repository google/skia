/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GraphiteDisplayParams_DEFINED
#define GraphiteDisplayParams_DEFINED

#include "include/gpu/graphite/ContextOptions.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/graphite/TestOptions.h"
#include "tools/window/DisplayParams.h"

namespace skwindow {

struct GraphiteTestOptions {
    GraphiteTestOptions() { fTestOptions.fContextOptions.fOptionsPriv = &fPriv; }

    GraphiteTestOptions(const GraphiteTestOptions& other)
            : fTestOptions(other.fTestOptions), fPriv(other.fPriv) {
        fTestOptions.fContextOptions.fOptionsPriv = &fPriv;
    }

    GraphiteTestOptions& operator=(const GraphiteTestOptions& other) {
        fTestOptions = other.fTestOptions;
        fPriv = other.fPriv;
        fTestOptions.fContextOptions.fOptionsPriv = &fPriv;
        return *this;
    }

    skiatest::graphite::TestOptions fTestOptions;
    skgpu::graphite::ContextOptionsPriv fPriv;
};

class GraphiteDisplayParams : public DisplayParams {
public:
    GraphiteDisplayParams(GraphiteTestOptions opts) : DisplayParams(), fGraphiteTestOptions(opts) {}

    GraphiteDisplayParams(const DisplayParams* other) : DisplayParams(other) {
        if (auto existing = other->graphiteTestOptions()) {
            fGraphiteTestOptions = *existing;
        } else {
            fGraphiteTestOptions = GraphiteTestOptions();
        }
    }

    std::unique_ptr<DisplayParams> clone() const override {
        return std::make_unique<GraphiteDisplayParams>(*this);
    }

    const GraphiteTestOptions* graphiteTestOptions() const override {
        return &fGraphiteTestOptions;
    }

private:
    friend class GraphiteDisplayParamsBuilder;

    GraphiteTestOptions fGraphiteTestOptions;
};

class GraphiteDisplayParamsBuilder : public DisplayParamsBuilder {
public:
    GraphiteDisplayParamsBuilder()
            : DisplayParamsBuilder(std::make_unique<GraphiteDisplayParams>(GraphiteTestOptions())) {
    }

    GraphiteDisplayParamsBuilder(const DisplayParams* other)
            : DisplayParamsBuilder(std::make_unique<GraphiteDisplayParams>(other)) {}

    GraphiteDisplayParamsBuilder& graphiteTestOptions(
            const GraphiteTestOptions& graphiteTestOptions) {
        SkASSERT_RELEASE(fDisplayParams);
        reinterpret_cast<GraphiteDisplayParams*>(fDisplayParams.get())->fGraphiteTestOptions =
                graphiteTestOptions;
        return *this;
    }
};

}  // namespace skwindow

#endif
