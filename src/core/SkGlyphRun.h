/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRun_DEFINED
#define SkGlyphRun_DEFINED

#include <functional>
#include <vector>

#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkSpan.h"
#include "src/core/SkZip.h"
#include "src/shaders/SkLocalMatrixShader.h"

class SkBaseDevice;
class SkGlyph;
class SkTextBlob;
class SkTextBlobRunIterator;

class SkGlyphRun {
public:
    SkGlyphRun() = default;
    SkGlyphRun(const SkFont& font,
               SkSpan<const SkPoint> positions,
               SkSpan<const SkGlyphID> glyphIDs,
               SkSpan<const char> text,
               SkSpan<const uint32_t> clusters,
               SkSpan<const SkVector> scaledRotations);

    SkGlyphRun(const SkGlyphRun& glyphRun, const SkFont& font);

    size_t runSize() const { return fSource.size(); }
    SkSpan<const SkPoint> positions() const { return fSource.get<1>(); }
    SkSpan<const SkGlyphID> glyphsIDs() const { return fSource.get<0>(); }
    SkZip<const SkGlyphID, const SkPoint> source() const { return fSource; }
    const SkFont& font() const { return fFont; }
    SkSpan<const uint32_t> clusters() const { return fClusters; }
    SkSpan<const char> text() const { return fText; }
    SkSpan<const SkVector> scaledRotations() const { return fScaledRotations; }

private:
    // GlyphIDs and positions.
    const SkZip<const SkGlyphID, const SkPoint> fSource;
    // Original text from SkTextBlob if present. Will be empty of not present.
    const SkSpan<const char> fText;
    // Original clusters from SkTextBlob if present. Will be empty if not present.
    const SkSpan<const uint32_t>   fClusters;
    // Possible RSXForm information
    const SkSpan<const SkVector> fScaledRotations;
    // Font for this run modified to have glyph encoding and left alignment.
    SkFont fFont;
};

class SkGlyphRunList {
    SkSpan<const SkGlyphRun> fGlyphRuns;

public:
    SkGlyphRunList();
    // Blob maybe null.
    SkGlyphRunList(
            const SkTextBlob* blob,
            SkPoint origin,
            SkSpan<const SkGlyphRun> glyphRunList);

    SkGlyphRunList(const SkGlyphRun& glyphRun);

    uint64_t uniqueID() const;
    bool anyRunsLCD() const;
    void temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID) const;

    bool canCache() const { return fOriginalTextBlob != nullptr; }
    size_t runCount() const { return fGlyphRuns.size(); }
    size_t totalGlyphCount() const {
        size_t glyphCount = 0;
        for (const SkGlyphRun& run : *this) {
            glyphCount += run.runSize();
        }
        return glyphCount;
    }

    bool hasRSXForm() const {
        for (const SkGlyphRun& run : *this) {
            if (!run.scaledRotations().empty()) { return true; }
        }
        return false;
    }

    bool allFontsFinite() const;

    SkPoint origin() const { return fOrigin; }
    const SkTextBlob* blob() const { return fOriginalTextBlob; }

    auto begin() -> decltype(fGlyphRuns.begin())               { return fGlyphRuns.begin();  }
    auto end()   -> decltype(fGlyphRuns.end())                 { return fGlyphRuns.end();    }
    auto begin() const -> decltype(fGlyphRuns.cbegin())        { return fGlyphRuns.cbegin(); }
    auto end()   const -> decltype(fGlyphRuns.cend())          { return fGlyphRuns.cend();   }
    auto size()  const -> decltype(fGlyphRuns.size())          { return fGlyphRuns.size();   }
    auto empty() const -> decltype(fGlyphRuns.empty())         { return fGlyphRuns.empty();  }
    auto operator [] (size_t i) const -> decltype(fGlyphRuns[i]) { return fGlyphRuns[i];     }

    // Breaks up a glyph run list into a bunch of glyph run lists without TSXForm.
    template<typename DrawGlyphRunList>
    static void DrawGlyphRunListWithTSXForm(const SkGlyphRunList& glyphRunList,
                                            const SkPaint& paint,
                                            DrawGlyphRunList drawGlyphRunList);

private:
    // The text blob is needed to hookup the call back that the SkTextBlob destructor calls. It
    // should be used for nothing else
    const SkTextBlob*  fOriginalTextBlob{nullptr};
    SkPoint fOrigin = {0, 0};
};

template<typename DrawGlyphRunList>
void SkGlyphRunList::DrawGlyphRunListWithTSXForm(const SkGlyphRunList& glyphRunList,
                                                 const SkPaint& paint,
                                                 DrawGlyphRunList drawGlyphRunList) {
    // TODO: This does not work for arbitrary shader DAGs
    //      (when there is no single leaf local matrix).
    // What we really need is proper post-LM plumbing for shaders.
    auto makePostInverseLM = [] (const SkShader* shader, const SkMatrix& m) ->  sk_sp<SkShader> {
        SkMatrix inverse;
        if (!shader || !m.invert(&inverse)) {
            return nullptr;
        }

        // Normal LMs pre-compose.  In order to push a post local matrix, we shoot for
        // something along these lines (where all new components are pre-composed):
        //
        //   new_lm X current_lm == current_lm X inv(current_lm) X new_lm X current_lm
        //
        // We also have two sources of local matrices:
        //   - the actual shader lm
        //   - outer lms applied via SkLocalMatrixShader

        SkMatrix outer_lm;
        const auto nested_shader = as_SB(shader)->makeAsALocalMatrixShader(&outer_lm);
        if (nested_shader) {
            // unfurl the shader
            shader = nested_shader.get();
        } else {
            outer_lm.reset();
        }

        const auto lm = *as_SB(shader)->totalLocalMatrix(nullptr);
        SkMatrix lm_inv;
        if (!lm.invert(&lm_inv)) {
            return nullptr;
        }

        // Note: since we unfurled the shader above, we don't need to apply an outer_lm inverse
        return shader->makeWithLocalMatrix(lm_inv * inverse * lm * outer_lm);
    };

    for (const SkGlyphRun& run : glyphRunList) {
        if (run.scaledRotations().empty()) {
            drawGlyphRunList(SkGlyphRunList{run}, paint, SkMatrix::I());
        } else {
            SkPoint origin = glyphRunList.origin();
            SkPoint sharedPos{0, 0};    // we're at the origin
            SkGlyphID sharedGlyphID;
            SkGlyphRun glyphRun {
                    run.font(),
                    SkSpan<const SkPoint>{&sharedPos, 1},
                    SkSpan<const SkGlyphID>{&sharedGlyphID, 1},
                    SkSpan<const char>{},
                    SkSpan<const uint32_t>{},
                    SkSpan<const SkVector>{}
            };

            for (auto [i, glyphID, pos] : SkMakeEnumerate(run.source())) {
                sharedGlyphID = glyphID;
                auto [scos, ssin] = run.scaledRotations()[i];
                SkRSXform rsxForm = SkRSXform::Make(scos, ssin, pos.x(), pos.y());
                SkMatrix glyphToLocal;
                glyphToLocal.setRSXform(rsxForm).postTranslate(origin.x(), origin.y());

                // We want to rotate each glyph by the rsxform, but we don't want to rotate "space"
                // (i.e. the shader that cares about the ctm) so we have to undo our little ctm
                // trick with a localmatrixshader so that the shader draws as if there was no
                // change to the ctm.
                SkPaint transformingPaint{paint};
                transformingPaint.setShader(makePostInverseLM(paint.getShader(), glyphToLocal));

                drawGlyphRunList(SkGlyphRunList{glyphRun}, transformingPaint, glyphToLocal);
            }
        }
    }
}

class SkGlyphRunBuilder {
public:
    const SkGlyphRunList& textToGlyphRunList(
            const SkFont&, const void* bytes, size_t byteLength, SkPoint origin);
    const SkGlyphRunList& blobToGlyphRunList(const SkTextBlob& blob, SkPoint origin);

    bool empty() const { return fGlyphRunListStorage.empty(); }

private:
    void initialize(int totalRunSize);
    void initialize(const SkTextBlob& blob);
    SkSpan<const SkGlyphID> textToGlyphIDs(
            const SkFont& font, const void* bytes, size_t byteLength, SkTextEncoding);

    void makeGlyphRun(
            const SkFont& font,
            SkSpan<const SkGlyphID> glyphIDs,
            SkSpan<const SkPoint> positions,
            SkSpan<const char> text,
            SkSpan<const uint32_t> clusters,
            SkSpan<const SkVector> scaledRotations);

    const SkGlyphRunList& makeGlyphRunList(const SkTextBlob* blob, SkPoint origin);

    int fMaxTotalRunSize{0};
    SkAutoTMalloc<SkPoint> fPositions;
    std::vector<SkVector> fScaledRotations;

    std::vector<SkGlyphRun> fGlyphRunListStorage;
    SkGlyphRunList fGlyphRunList;

    // Used as a temporary for preparing using utfN text. This implies that only one run of
    // glyph ids will ever be needed because blobs are already glyph based.
    std::vector<SkGlyphID> fScratchGlyphIDs;
};

#endif  // SkGlyphRun_DEFINED
