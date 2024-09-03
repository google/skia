/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGTextPriv_DEFINED
#define SkSVGTextPriv_DEFINED

#include "include/core/SkContourMeasure.h"
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/base/SkTLazy.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <vector>

class SkSVGLengthContext;
class SkSVGRenderContext;
class SkSVGTextContainer;
class SkSVGTextPath;
class SkString;
class SkTextBlob;
enum class SkSVGXmlSpace;
struct SkRSXform;

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
    using ShapedTextCallback = std::function<void(const SkSVGRenderContext&,
                                                  const sk_sp<SkTextBlob>&,
                                                  const SkPaint*,
                                                  const SkPaint*)>;

    // Helper for encoding optional positional attributes.
    class PosAttrs {
    public:
        // TODO: rotate
        enum Attr : size_t {
            kX      = 0,
            kY      = 1,
            kDx     = 2,
            kDy     = 3,
            kRotate = 4,
        };

        float  operator[](Attr a) const { return fStorage[a]; }
        float& operator[](Attr a)       { return fStorage[a]; }

        bool has(Attr a) const { return fStorage[a] != kNone; }
        bool hasAny()    const {
            return this->has(kX)
                || this->has(kY)
                || this->has(kDx)
                || this->has(kDy)
                || this->has(kRotate);
        }

        void setImplicitRotate(bool imp) { fImplicitRotate = imp; }
        bool isImplicitRotate() const { return fImplicitRotate; }

    private:
        inline static constexpr auto kNone = std::numeric_limits<float>::infinity();

        float fStorage[5]     = { kNone, kNone, kNone, kNone, kNone };
        bool  fImplicitRotate = false;
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
        SkSVGTextContext*         fTextContext;
        const ScopedPosResolver*  fParent;          // parent resolver (fallback)
        const size_t              fCharIndexOffset; // start index for the current resolver
        const std::vector<float>  fX,
                                  fY,
                                  fDx,
                                  fDy;
        const std::vector<float>& fRotate;

        // cache for the last known index with explicit positioning
        mutable size_t           fLastPosIndex = std::numeric_limits<size_t>::max();

    };

    SkSVGTextContext(const SkSVGRenderContext&,
                     const ShapedTextCallback&,
                     const SkSVGTextPath* = nullptr);
    ~SkSVGTextContext() override;

    // Shape and queue codepoints for final alignment.
    void shapeFragment(const SkString&, const SkSVGRenderContext&, SkSVGXmlSpace);

    // Perform final adjustments and push shaped blobs to the callback.
    void flushChunk(const SkSVGRenderContext& ctx);

    const ShapedTextCallback& getCallback() const { return fCallback; }

private:
    struct PositionAdjustment {
        SkVector offset;
        float    rotation;
    };

    struct ShapeBuffer {
        skia_private::STArray<128, char              , true> fUtf8;
        // per-utf8-char cumulative pos adjustments
        skia_private::STArray<128, PositionAdjustment, true> fUtf8PosAdjust;

        void reserve(size_t size) {
            fUtf8.reserve_exact(fUtf8.size() + SkToInt(size));
            fUtf8PosAdjust.reserve_exact(fUtf8PosAdjust.size() + SkToInt(size));
        }

        void reset() {
            fUtf8.clear();
            fUtf8PosAdjust.clear();
        }

        void append(SkUnichar, PositionAdjustment);
    };

    struct RunRec {
        SkFont                                font;
        std::unique_ptr<SkPaint>              fillPaint,
                                              strokePaint;
        std::unique_ptr<SkGlyphID[]>          glyphs;        // filled by SkShaper
        std::unique_ptr<SkPoint[]>            glyphPos;      // filled by SkShaper
        std::unique_ptr<PositionAdjustment[]> glyhPosAdjust; // deferred positioning adjustments
        size_t                                glyphCount;
        SkVector                              advance;
    };

    // Caches path information to accelerate position lookups.
    class PathData {
    public:
        PathData(const SkSVGRenderContext&, const SkSVGTextPath&);

        SkMatrix getMatrixAt(float offset) const;

        float length() const { return fLength; }

    private:
        std::vector<sk_sp<SkContourMeasure>> fContours;
        float                                fLength = 0; // total path length
    };

    void shapePendingBuffer(const SkSVGRenderContext&, const SkFont&);

    SkRSXform computeGlyphXform(SkGlyphID, const SkFont&, const SkPoint& glyph_pos,
                                const PositionAdjustment&) const;

    // SkShaper callbacks
    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    Buffer runBuffer(const RunInfo& ri) override;
    void commitRunBuffer(const RunInfo& ri) override;
    void commitLine() override;

    // http://www.w3.org/TR/SVG11/text.html#TextLayout
    const SkSVGRenderContext&       fRenderContext; // original render context
    const ShapedTextCallback&       fCallback;
    std::unique_ptr<SkShaper>       fShaper;
    std::vector<RunRec>             fRuns;
    const ScopedPosResolver*        fPosResolver = nullptr;
    std::unique_ptr<PathData>       fPathData;

    // shaper state
    ShapeBuffer                     fShapeBuffer;
    std::vector<uint32_t>           fShapeClusterBuffer;

    // chunk state
    SkPoint                         fChunkPos     = {0,0}; // current text chunk position
    SkVector                        fChunkAdvance = {0,0}; // cumulative advance
    float                           fChunkAlignmentFactor; // current chunk alignment

    // tracks the global text subtree char index (cross chunks).  Used for position resolution.
    size_t                          fCurrentCharIndex = 0;

    // cached for access from SkShaper callbacks.
    SkTLazy<SkPaint>                fCurrentFill;
    SkTLazy<SkPaint>                fCurrentStroke;

    bool                            fPrevCharSpace = true; // WS filter state
    bool                            fForcePrimitiveShaping = false;
};

#endif // SkSVGTextPriv_DEFINED
