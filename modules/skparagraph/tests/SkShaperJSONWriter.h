/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaperJSONWriter_DEFINED
#define SkShaperJSONWriter_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "modules/skshaper/include/SkShaper.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class SkJSONWriter;
template <typename T> class SkSpan;

class SkShaperJSONWriter final : public SkShaper::RunHandler {
public:
    SkShaperJSONWriter(SkJSONWriter* JSONWriter, const char* utf8, size_t size);

    void beginLine() override;
    void runInfo(const RunInfo& info) override;
    void commitRunInfo() override;

    Buffer runBuffer(const RunInfo& info) override;

    void commitRunBuffer(const RunInfo& info) override;

    void commitLine() override {}

    using BreakupClustersCallback =
            std::function<void(size_t, size_t, uint32_t, uint32_t)>;

    // Break up cluster into a set of ranges for the UTF8, and the glyphIDs.
    static void BreakupClusters(size_t utf8Begin, size_t utf8End,
                                SkSpan<const uint32_t> clusters,
                                const BreakupClustersCallback& processMToN);


    using VisualizeClustersCallback =
        std::function<void(size_t, SkSpan<const char>, SkSpan<const SkGlyphID>)>;

    // Gather runs of 1:1 into larger runs, and display M:N as single entries.
    static void VisualizeClusters(const char utf8[],
                                  size_t utf8Begin, size_t utf8End,
                                  SkSpan<const SkGlyphID> glyphIDs,
                                  SkSpan<const uint32_t> clusters,
                                  const VisualizeClustersCallback& processMToN);

private:
    void displayMToN(size_t codePointCount,
                     SkSpan<const char> utf8,
                     SkSpan<const SkGlyphID> glyphIDs);

    SkJSONWriter* fJSONWriter;
    std::vector<SkGlyphID> fGlyphs;
    std::vector<SkPoint> fPositions;
    std::vector<uint32_t> fClusters;

    std::string fUTF8;
};

#endif  // SkShaperJSONWriter_DEFINED
