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

class GraphiteDisplayParams : public DisplayParams {
public:
    GraphiteDisplayParams(const skiatest::graphite::TestOptions& opts)
            : DisplayParams(), fGraphiteTestOptions(opts) {}

    GraphiteDisplayParams(const DisplayParams* other)
            : DisplayParams(other)
            , fGraphiteTestOptions(other->graphiteTestOptions()
                                           ? *other->graphiteTestOptions()
                                           : skiatest::graphite::TestOptions()) {}

    std::unique_ptr<DisplayParams> clone() const override {
        return std::make_unique<GraphiteDisplayParams>(*this);
    }

    const skiatest::graphite::TestOptions* graphiteTestOptions() const override {
        return &fGraphiteTestOptions;
    }

private:
    friend class GraphiteDisplayParamsBuilder;

    skiatest::graphite::TestOptions fGraphiteTestOptions;
};

class GraphiteDisplayParamsBuilder : public DisplayParamsBuilder {
public:
    GraphiteDisplayParamsBuilder()
            : DisplayParamsBuilder(
                      std::make_unique<GraphiteDisplayParams>(skiatest::graphite::TestOptions())) {}

    GraphiteDisplayParamsBuilder(const DisplayParams* other)
            : DisplayParamsBuilder(std::make_unique<GraphiteDisplayParams>(other)) {}

    GraphiteDisplayParamsBuilder& graphiteTestOptions(
            const skiatest::graphite::TestOptions& graphiteTestOptions) {
        SkASSERT_RELEASE(fDisplayParams);
        reinterpret_cast<GraphiteDisplayParams*>(fDisplayParams.get())->fGraphiteTestOptions =
                graphiteTestOptions;
        return *this;
    }
};

}  // namespace skwindow

#endif
