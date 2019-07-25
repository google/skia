// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef run_handler_DEFINED
#define run_handler_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypes.h"
#include "modules/skshaper/include/SkShaper.h"

#include <vector>

class SkFont;

namespace editor {

class RunHandler final : public SkShaper::RunHandler {
public:
    RunHandler(const char* utf8Text, size_t) : fUtf8Text(utf8Text) {}
    using RunCallback = void (*)(void* context,
                                 const char* utf8Text,
                                  size_t utf8TextBytes,
                                  size_t glyphCount,
                                  const SkGlyphID* glyphs,
                                  const SkPoint* positions,
                                  const uint32_t* clusters,
                                  const SkFont& font);
    void setRunCallback(RunCallback f, void* context) {
        fCallbackContext = context;
        fCallbackFunction = f;
    }

    sk_sp<SkTextBlob> makeBlob();
    SkPoint endPoint() const { return fOffset; }
    float yOffset() const { return fOffset.y() - (fMaxRunDescent + fMaxRunLeading - fMaxRunAscent); }

    void beginLine() override;
    void runInfo(const RunInfo&) override;
    void commitRunInfo() override;
    SkShaper::RunHandler::Buffer runBuffer(const RunInfo&) override;
    void commitRunBuffer(const RunInfo&) override;
    void commitLine() override;

    const std::vector<unsigned>& lineEndOffsets() const { return fLineEndOffsets; }

private:
    SkTextBlobBuilder fBuilder;
    std::vector<unsigned> fLineEndOffsets;
    const SkGlyphID* fCurrentGlyphs = nullptr;
    const SkPoint* fCurrentPoints = nullptr;
    void* fCallbackContext = nullptr;
    RunCallback fCallbackFunction = nullptr;
    char const * const fUtf8Text;
    unsigned fTextOffset = 0;
    uint32_t* fClusters = nullptr;
    int fClusterOffset = 0;
    int fGlyphCount = 0;
    SkScalar fMaxRunAscent = 0;
    SkScalar fMaxRunDescent = 0;
    SkScalar fMaxRunLeading = 0;
    SkPoint fCurrentPosition = {0, 0};
    SkPoint fOffset = {0, 0};
};
}  // namespace editor
#endif  // run_handler_DEFINED
