/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRasterPipelineVizualizer_DEFINED
#define SkRasterPipelineVizualizer_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkRefCnt.h"  // IWYU pragma: keep

#include <vector>

class SkArenaAlloc;
class SkBlitter;
class SkMatrix;
class SkPaint;
class SkPixmap;
class SkShader;
class SkSurfaceProps;
enum class SkRasterPipelineOp;

namespace SkRasterPipelineVisualizer {

// After each stage, visualize 0 or more (probably 1-4) lanes into the provided panels.
// The vectors are parallel data structures with the op chosen by the user of what
// and how to be visualized. The number of panels per stage can differ (the pipeline
// frequently can be thought of as a ragged 2 dimensional array of images).
struct DebugStage {
    std::vector<SkBitmap> panels;
    // Should be only ops that start with debug_
    std::vector<SkRasterPipelineOp> ops;
};

// The same as SkCreateRasterPipelineBlitter but takes in some stages to
// visualize the output of each of the main stages. This will inject some
// stages into the shader on the provided paint and then return a blitter
// which can be used to draw the final output and the intermediate represenations.
// If stages is not the same size as the actual number of pipeline stages, this
// will abort. The first time this function is called, the original pipeline will be
// displayed via standard out to make it easier to get the number of stages right.
//
// Note that different methods on the provided Blitter may add stages after creation
// (e.g. for blending with the destination) so this won't visualize those, but the main
// ones from the shader should be visualized via this code path.
SkBlitter* CreateBlitter(const SkPixmap& output,
                         const std::vector<DebugStage>& stages,
                         const SkPaint&,
                         const SkMatrix& ctm,
                         SkArenaAlloc*,
                         sk_sp<SkShader> clipShader,
                         const SkSurfaceProps& props);

// A convinence class for making many debug stages. The user chooses which lanes are
// useful to be visualized for each lane.
//   SkBitmap* panels = ...;  // make a bunch of bitmaps somehow
//
//   DebugStageBuilder stageBuilder;
//   stageBuilder.add(panels[0], SkRasterPipelineOp::debug_x,
//                    panels[1], SkRasterPipelineOp::debug_y)
//               .add(panels[2], SkRasterPipelineOp::debug_b)
//               .add(panels[3], SkRasterPipelineOp::debug_r_255,
//                    panels[4], SkRasterPipelineOp::debug_g_255,
//                    panels[5], SkRasterPipelineOp::debug_b_255,
//                    panels[6], SkRasterPipelineOp::debug_a_255);
//   auto stages = stageBuilder.build()
class DebugStageBuilder {
public:
    DebugStageBuilder() = default;
    DebugStageBuilder(const DebugStageBuilder&) = delete;
    DebugStageBuilder(DebugStageBuilder&&) = delete;
    DebugStageBuilder& operator=(const DebugStageBuilder&) = delete;
    DebugStageBuilder& operator=(DebugStageBuilder&&) = delete;

    template <typename... Args>
    DebugStageBuilder& add(const SkBitmap& panel, SkRasterPipelineOp op, Args... args) {
        std::vector<SkBitmap> panels;
        std::vector<SkRasterPipelineOp> ops;

        add_next(panels, ops, panel, op, args...);
        fDebugStages.push_back({panels, ops});
        return *this;
    }

    DebugStageBuilder& add() {
        std::vector<SkBitmap> panels;
        std::vector<SkRasterPipelineOp> ops;
        fDebugStages.push_back({panels, ops});
        return *this;
    }

    std::vector<DebugStage> build() { return fDebugStages; }

private:
    std::vector<DebugStage> fDebugStages;

    static void add_next(std::vector<SkBitmap>& v, std::vector<SkRasterPipelineOp>& ops) {}

    template <typename... Args>
    static void add_next(std::vector<SkBitmap>& panels,
                         std::vector<SkRasterPipelineOp>& ops,
                         const SkBitmap& panel,
                         SkRasterPipelineOp op,
                         Args... args) {
        panels.emplace_back(panel);
        ops.emplace_back(op);
        add_next(panels, ops, args...);
    }
};

}  // namespace SkRasterPipelineVisualizer

#endif
