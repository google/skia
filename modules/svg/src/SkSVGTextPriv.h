/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGTextPriv_DEFINED
#define SkSVGTextPriv_DEFINED

#include "modules/skshaper/include/SkShaper.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGText.h"

// SkSVGTextContext is responsible for sequencing input text chars into "chunks".
// A single text chunk can span multiple structural elements (<text>, <tspan>, etc),
// and per [1] new chunks are emitted
//
//   a) for each top level text element (<text>, <textPath>)
//   b) whenever a character with an explicit absolute position is encountered
//
// The implementation queues shaped run data until a full text chunk is resolved, at which
// point we have enough information to perform final alignment and rendering.
//
// [1] https://www.w3.org/TR/SVG11/text.html#TextLayoutIntroduction
class SkSVGTextContext final : SkShaper::RunHandler {
public:

    // Helper for encoding optional positional attributes.
    class PosAttrs {
    public:
        // TODO: dx, dy, rotate
        enum Attr : size_t {
            kX = 0,
            kY = 1,
        };

        float  operator[](Attr a) const { return fStorage[a]; }
        float& operator[](Attr a)       { return fStorage[a]; }

        bool has(Attr a) const { return fStorage[a] != kNone; }
        bool hasAny()    const { return this->has(kX) || this->has(kY); }

    private:
        static constexpr auto kNone = std::numeric_limits<float>::infinity();

        float fStorage[2] = { kNone, kNone };
    };

    // Helper for cascading position attribute resolution (x, y, dx, dy, rotate) [1]:
    //   - each text position element can specify an arbitrary-length attribute array
    //   - for each character, we look up a given attribute first in its local attribute array,
    //     then in the ancestor chain (cascading/fallback) - and return the first value encountered.
    //   - the lookup is based on character index relative to the text content subtree
    //     (i.e. the index crosses chunk boundaries)
    //
    // [1] https://www.w3.org/TR/SVG11/text.html#TSpanElementXAttribute
    class ScopedPosResolver {
    public:
        ScopedPosResolver(const SkSVGTextContainer&, const SkSVGLengthContext&, SkSVGTextContext*,
                          size_t);

        ScopedPosResolver(const SkSVGTextContainer&, const SkSVGLengthContext&, SkSVGTextContext*);

        ~ScopedPosResolver();

        PosAttrs resolve(size_t charIndex) const;

    private:
        SkSVGTextContext*        fTextContext;
        const ScopedPosResolver* fParent;          // parent resolver (fallback)
        const size_t             fCharIndexOffset; // start index for the current resolver
        const std::vector<float> fX,
                                 fY;

        // cache for the last known index with explicit positioning
        mutable size_t           fLastPosIndex = std::numeric_limits<size_t>::max();

    };

    SkSVGTextContext(const SkSVGPresentationContext&, sk_sp<SkFontMgr>);

    // Queues codepoints for rendering.
    void appendFragment(const SkString&, const SkSVGRenderContext&, SkSVGXmlSpace);

    // Perform actual rendering for queued codepoints.
    void flushChunk(const SkSVGRenderContext& ctx);

private:
    struct RunRec {
        SkFont                       font;
        std::unique_ptr<SkPaint>     fillPaint,
                                     strokePaint;
        std::unique_ptr<SkGlyphID[]> glyphs;
        std::unique_ptr<SkPoint[]>   glyphPos;
        size_t                       glyphCount;
        SkVector                     advance;
    };

    // SkShaper callbacks
    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    Buffer runBuffer(const RunInfo& ri) override;
    void commitRunBuffer(const RunInfo& ri) override;
    void commitLine() override {}

    // http://www.w3.org/TR/SVG11/text.html#TextLayout
    const std::unique_ptr<SkShaper> fShaper;
    std::vector<RunRec>             fRuns;
    const ScopedPosResolver*        fPosResolver = nullptr;

    SkPoint                         fChunkPos;             // current text chunk position
    SkVector                        fChunkAdvance = {0,0}; // cumulative advance
    float                           fChunkAlignmentFactor; // current chunk alignment

    // tracks the global text subtree char index (cross chunks).  Used for position resolution.
    size_t                          fCurrentCharIndex = 0;

    // cached for access from SkShaper callbacks.
    const SkPaint*                  fCurrentFill;
    const SkPaint*                  fCurrentStroke;

    bool                            fPrevCharSpace = true; // WS filter state
};

#endif // SkSVGTextPriv_DEFINED
