/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/SubRunContainer.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTLogic.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkZip.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/AtlasTypes.h"
#include "src/text/GlyphRun.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/SDFMaskFilter.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/SubRunControl.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <new>
#include <optional>
#include <vector>

using namespace skia_private;
using namespace skglyph;

// -- GPU Text -------------------------------------------------------------------------------------
// Naming conventions
//  * drawMatrix - the CTM from the canvas.
//  * drawOrigin - the x, y location of the drawTextBlob call.
//  * positionMatrix - this is the combination of the drawMatrix and the drawOrigin:
//        positionMatrix = drawMatrix * TranslationMatrix(drawOrigin.x, drawOrigin.y);
//
// Note:
//   In order to transform Slugs, you need to set the fSupportBilerpFromGlyphAtlas on
//   GrContextOptions.

namespace sktext::gpu {
// -- SubRunStreamTag ------------------------------------------------------------------------------
enum SubRun::SubRunStreamTag : int {
    kBad = 0,  // Make this 0 to line up with errors from readInt.
    kDirectMaskStreamTag,
#if !defined(SK_DISABLE_SDF_TEXT)
    kSDFTStreamTag,
#endif
    kTransformMaskStreamTag,
    kPathStreamTag,
    kDrawableStreamTag,
    kSubRunStreamTagCount,
};

}  // namespace sktext::gpu

using MaskFormat = skgpu::MaskFormat;

using namespace sktext;
using namespace sktext::gpu;

namespace {
SkSpan<const SkPackedGlyphID> get_packedIDs(SkZip<const SkPackedGlyphID, const SkPoint> accepted) {
    return accepted.get<0>();
}

SkSpan<const SkGlyphID> get_glyphIDs(SkZip<const SkGlyphID, const SkPoint> accepted) {
    return accepted.get<0>();
}

template <typename U>
SkSpan<const SkPoint> get_positions(SkZip<U, const SkPoint> accepted) {
    return accepted.template get<1>();
}

// -- PathOpSubmitter ------------------------------------------------------------------------------
// PathOpSubmitter holds glyph ids until ready to draw. During drawing, the glyph ids are
// converted to SkPaths. PathOpSubmitter can only be serialized when it is holding glyph ids;
// it can only be serialized before submitDraws has been called.
class PathOpSubmitter {
public:
    PathOpSubmitter() = delete;
    PathOpSubmitter(const PathOpSubmitter&) = delete;
    const PathOpSubmitter& operator=(const PathOpSubmitter&) = delete;
    PathOpSubmitter(PathOpSubmitter&& that)
            // Transfer ownership of fIDsOrPaths from that to this.
            : fIDsOrPaths{std::exchange(
                      const_cast<SkSpan<IDOrPath>&>(that.fIDsOrPaths), SkSpan<IDOrPath>{})}
            , fPositions{that.fPositions}
            , fStrikeToSourceScale{that.fStrikeToSourceScale}
            , fIsAntiAliased{that.fIsAntiAliased}
            , fStrikePromise{std::move(that.fStrikePromise)} {}
    PathOpSubmitter& operator=(PathOpSubmitter&& that) {
        this->~PathOpSubmitter();
        new (this) PathOpSubmitter{std::move(that)};
        return *this;
    }
    PathOpSubmitter(bool isAntiAliased,
                    SkScalar strikeToSourceScale,
                    SkSpan<SkPoint> positions,
                    SkSpan<IDOrPath> idsOrPaths,
                    SkStrikePromise&& strikePromise);

    ~PathOpSubmitter();

    static PathOpSubmitter Make(SkZip<const SkGlyphID, const SkPoint> accepted,
                                bool isAntiAliased,
                                SkScalar strikeToSourceScale,
                                SkStrikePromise&& strikePromise,
                                SubRunAllocator* alloc);

    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;
    static std::optional<PathOpSubmitter> MakeFromBuffer(SkReadBuffer& buffer,
                                                         SubRunAllocator* alloc,
                                                         const SkStrikeClient* client);

    // submitDraws is not thread safe. It only occurs the single thread drawing portion of the GPU
    // rendering.
    void submitDraws(SkCanvas*,
                     SkPoint drawOrigin,
                     const SkPaint& paint) const;

private:
    // When PathOpSubmitter is created only the glyphIDs are needed, during the submitDraws call,
    // the glyphIDs are converted to SkPaths.
    const SkSpan<IDOrPath> fIDsOrPaths;
    const SkSpan<const SkPoint> fPositions;
    const SkScalar fStrikeToSourceScale;
    const bool fIsAntiAliased;

    mutable SkStrikePromise fStrikePromise;
    mutable SkOnce fConvertIDsToPaths;
    mutable bool fPathsAreCreated{false};
};

int PathOpSubmitter::unflattenSize() const {
    return fPositions.size_bytes() + fIDsOrPaths.size_bytes();
}

void PathOpSubmitter::flatten(SkWriteBuffer& buffer) const {
    fStrikePromise.flatten(buffer);

    buffer.writeInt(fIsAntiAliased);
    buffer.writeScalar(fStrikeToSourceScale);
    buffer.writePointArray(fPositions);
    for (IDOrPath& idOrPath : fIDsOrPaths) {
        buffer.writeInt(idOrPath.fGlyphID);
    }
}

std::optional<PathOpSubmitter> PathOpSubmitter::MakeFromBuffer(SkReadBuffer& buffer,
                                                               SubRunAllocator* alloc,
                                                               const SkStrikeClient* client) {
    std::optional<SkStrikePromise> strikePromise =
            SkStrikePromise::MakeFromBuffer(buffer, client, SkStrikeCache::GlobalStrikeCache());
    if (!buffer.validate(strikePromise.has_value())) {
        return std::nullopt;
    }

    bool isAntiAlias = buffer.readInt();

    SkScalar strikeToSourceScale = buffer.readScalar();
    if (!buffer.validate(0 < strikeToSourceScale)) { return std::nullopt; }

    SkSpan<SkPoint> positions = MakePointsFromBuffer(buffer, alloc);
    if (positions.empty()) { return std::nullopt; }
    const int glyphCount = SkCount(positions);

    // Remember, we stored an int for glyph id.
    if (!buffer.validateCanReadN<int>(glyphCount)) { return std::nullopt; }
    auto idsOrPaths = SkSpan(alloc->makeUniqueArray<IDOrPath>(glyphCount).release(), glyphCount);
    for (auto& idOrPath : idsOrPaths) {
        idOrPath.fGlyphID = SkTo<SkGlyphID>(buffer.readInt());
    }

    if (!buffer.isValid()) { return std::nullopt; }

    return PathOpSubmitter{isAntiAlias,
                           strikeToSourceScale,
                           positions,
                           idsOrPaths,
                           std::move(strikePromise.value())};
}

PathOpSubmitter::PathOpSubmitter(
        bool isAntiAliased,
        SkScalar strikeToSourceScale,
        SkSpan<SkPoint> positions,
        SkSpan<IDOrPath> idsOrPaths,
        SkStrikePromise&& strikePromise)
        : fIDsOrPaths{idsOrPaths}
        , fPositions{positions}
        , fStrikeToSourceScale{strikeToSourceScale}
        , fIsAntiAliased{isAntiAliased}
        , fStrikePromise{std::move(strikePromise)} {
    SkASSERT(!fPositions.empty());
}

PathOpSubmitter::~PathOpSubmitter() {
    // If we have converted glyph IDs to paths, then clean up the SkPaths.
    if (fPathsAreCreated) {
        for (auto& idOrPath : fIDsOrPaths) {
            idOrPath.fPath.~SkPath();
        }
    }
}

PathOpSubmitter PathOpSubmitter::Make(SkZip<const SkGlyphID, const SkPoint> accepted,
                                      bool isAntiAliased,
                                      SkScalar strikeToSourceScale,
                                      SkStrikePromise&& strikePromise,
                                      SubRunAllocator* alloc) {
    auto mapToIDOrPath = [](SkGlyphID glyphID) { return IDOrPath{glyphID}; };

    IDOrPath* const rawIDsOrPaths =
            alloc->makeUniqueArray<IDOrPath>(get_glyphIDs(accepted), mapToIDOrPath).release();

    return PathOpSubmitter{isAntiAliased,
                           strikeToSourceScale,
                           alloc->makePODSpan(get_positions(accepted)),
                           SkSpan(rawIDsOrPaths, accepted.size()),
                           std::move(strikePromise)};
}

void
PathOpSubmitter::submitDraws(SkCanvas* canvas, SkPoint drawOrigin, const SkPaint& paint) const {
    // Convert the glyph IDs to paths if it hasn't been done yet. This is thread safe.
    fConvertIDsToPaths([&]() {
        if (SkStrike* strike = fStrikePromise.strike()) {
            strike->glyphIDsToPaths(fIDsOrPaths);

            // Drop ref to strike so that it can be purged from the cache if needed.
            fStrikePromise.resetStrike();
            fPathsAreCreated = true;
        }
    });

    SkPaint runPaint{paint};
    runPaint.setAntiAlias(fIsAntiAliased);

    SkMaskFilterBase* maskFilter = as_MFB(runPaint.getMaskFilter());

    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(fStrikeToSourceScale, fStrikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());

    // If there are shaders, non-blur mask filters or styles, the path must be scaled into source
    // space independently of the CTM. This allows the CTM to be correct for the different effects.
    SkStrokeRec style(runPaint);
    bool needsExactCTM = runPaint.getShader()
                         || runPaint.getPathEffect()
                         || (!style.isFillStyle() && !style.isHairlineStyle())
                         || (maskFilter != nullptr && !maskFilter->asABlur(nullptr));
    if (!needsExactCTM) {
        SkMaskFilterBase::BlurRec blurRec;

        // If there is a blur mask filter, then sigma needs to be adjusted to account for the
        // scaling of fStrikeToSourceScale.
        if (maskFilter != nullptr && maskFilter->asABlur(&blurRec)) {
            runPaint.setMaskFilter(
                    SkMaskFilter::MakeBlur(blurRec.fStyle, blurRec.fSigma / fStrikeToSourceScale));
        }
        for (auto [idOrPath, pos] : SkMakeZip(fIDsOrPaths, fPositions)) {
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(pathMatrix);
            canvas->drawPath(idOrPath.fPath, runPaint);
        }
    } else {
        // Transform the path to device because the deviceMatrix must be unchanged to
        // draw effect, filter or shader paths.
        for (auto [idOrPath, pos] : SkMakeZip(fIDsOrPaths, fPositions)) {
            // Transform the glyph to source space.
            SkMatrix pathMatrix = strikeToSource;
            pathMatrix.postTranslate(pos.x(), pos.y());

            SkPath deviceOutline;
            idOrPath.fPath.transform(pathMatrix, &deviceOutline);
            deviceOutline.setIsVolatile(true);
            canvas->drawPath(deviceOutline, runPaint);
        }
    }
}

// -- PathSubRun -----------------------------------------------------------------------------------
class PathSubRun final : public SubRun {
public:
    PathSubRun(PathOpSubmitter&& pathDrawing) : fPathDrawing(std::move(pathDrawing)) {}

    static SubRunOwner Make(SkZip<const SkGlyphID, const SkPoint> accepted,
                            bool isAntiAliased,
                            SkScalar strikeToSourceScale,
                            SkStrikePromise&& strikePromise,
                            SubRunAllocator* alloc) {
        return alloc->makeUnique<PathSubRun>(
            PathOpSubmitter::Make(
                    accepted, isAntiAliased, strikeToSourceScale, std::move(strikePromise), alloc));
    }

    void draw(SkCanvas* canvas,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt>,
              const AtlasDrawDelegate&) const override {
        fPathDrawing.submitDraws(canvas, drawOrigin, paint);
    }

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        return true;
    }
    const AtlasSubRun* testingOnly_atlasSubRun() const override { return nullptr; }
    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client);

protected:
    SubRunStreamTag subRunStreamTag() const override { return SubRunStreamTag::kPathStreamTag; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    PathOpSubmitter fPathDrawing;
};

int PathSubRun::unflattenSize() const {
    return sizeof(PathSubRun) + fPathDrawing.unflattenSize();
}

void PathSubRun::doFlatten(SkWriteBuffer& buffer) const {
    fPathDrawing.flatten(buffer);
}

SubRunOwner PathSubRun::MakeFromBuffer(SkReadBuffer& buffer,
                                       SubRunAllocator* alloc,
                                       const SkStrikeClient* client) {
    auto pathOpSubmitter = PathOpSubmitter::MakeFromBuffer(buffer, alloc, client);
    if (!buffer.validate(pathOpSubmitter.has_value())) { return nullptr; }
    return alloc->makeUnique<PathSubRun>(std::move(*pathOpSubmitter));
}

// -- DrawableOpSubmitter --------------------------------------------------------------------------
// Shared code for submitting GPU ops for drawing glyphs as drawables.
class DrawableOpSubmitter {
public:
    DrawableOpSubmitter() = delete;
    DrawableOpSubmitter(const DrawableOpSubmitter&) = delete;
    const DrawableOpSubmitter& operator=(const DrawableOpSubmitter&) = delete;
    DrawableOpSubmitter(DrawableOpSubmitter&& that)
        : fStrikeToSourceScale{that.fStrikeToSourceScale}
        , fPositions{that.fPositions}
        , fIDsOrDrawables{that.fIDsOrDrawables}
        , fStrikePromise{std::move(that.fStrikePromise)} {}
    DrawableOpSubmitter& operator=(DrawableOpSubmitter&& that) {
        this->~DrawableOpSubmitter();
        new (this) DrawableOpSubmitter{std::move(that)};
        return *this;
    }
    DrawableOpSubmitter(SkScalar strikeToSourceScale,
                        SkSpan<SkPoint> positions,
                        SkSpan<IDOrDrawable> idsOrDrawables,
                        SkStrikePromise&& strikePromise);

    static DrawableOpSubmitter Make(SkZip<const SkGlyphID, const SkPoint> accepted,
                                    SkScalar strikeToSourceScale,
                                    SkStrikePromise&& strikePromise,
                                    SubRunAllocator* alloc) {
        auto mapToIDOrDrawable = [](const SkGlyphID glyphID) { return IDOrDrawable{glyphID}; };

        return DrawableOpSubmitter{
            strikeToSourceScale,
            alloc->makePODSpan(get_positions(accepted)),
            alloc->makePODArray<IDOrDrawable>(get_glyphIDs(accepted), mapToIDOrDrawable),
            std::move(strikePromise)};
    }

    int unflattenSize() const;
    void flatten(SkWriteBuffer& buffer) const;
    static std::optional<DrawableOpSubmitter> MakeFromBuffer(SkReadBuffer& buffer,
                                                             SubRunAllocator* alloc,
                                                             const SkStrikeClient* client);
    void submitDraws(SkCanvas* canvas, SkPoint drawOrigin, const SkPaint& paint) const;

private:
    const SkScalar fStrikeToSourceScale;
    const SkSpan<SkPoint> fPositions;
    const SkSpan<IDOrDrawable> fIDsOrDrawables;
    // When the promise is converted to a strike it acts as the ref on the strike to keep the
    // SkDrawable data alive.
    mutable SkStrikePromise fStrikePromise;
    mutable SkOnce fConvertIDsToDrawables;
};

int DrawableOpSubmitter::unflattenSize() const {
    return fPositions.size_bytes() + fIDsOrDrawables.size_bytes();
}

void DrawableOpSubmitter::flatten(SkWriteBuffer& buffer) const {
    fStrikePromise.flatten(buffer);

    buffer.writeScalar(fStrikeToSourceScale);
    buffer.writePointArray(fPositions);
    for (IDOrDrawable idOrDrawable : fIDsOrDrawables) {
        buffer.writeInt(idOrDrawable.fGlyphID);
    }
}

std::optional<DrawableOpSubmitter> DrawableOpSubmitter::MakeFromBuffer(
        SkReadBuffer& buffer, SubRunAllocator* alloc, const SkStrikeClient* client) {
    std::optional<SkStrikePromise> strikePromise =
            SkStrikePromise::MakeFromBuffer(buffer, client, SkStrikeCache::GlobalStrikeCache());
    if (!buffer.validate(strikePromise.has_value())) {
        return std::nullopt;
    }

    SkScalar strikeToSourceScale = buffer.readScalar();
    if (!buffer.validate(0 < strikeToSourceScale)) { return std::nullopt; }

    SkSpan<SkPoint> positions = MakePointsFromBuffer(buffer, alloc);
    if (positions.empty()) { return std::nullopt; }
    const int glyphCount = SkCount(positions);

    if (!buffer.validateCanReadN<int>(glyphCount)) { return std::nullopt; }
    auto idsOrDrawables = alloc->makePODArray<IDOrDrawable>(glyphCount);
    for (int i = 0; i < SkToInt(glyphCount); ++i) {
        // Remember, we stored an int for glyph id.
        idsOrDrawables[i].fGlyphID = SkTo<SkGlyphID>(buffer.readInt());
    }

    if (!buffer.isValid()) {
        return std::nullopt;
    }

    return DrawableOpSubmitter{strikeToSourceScale,
                               positions,
                               SkSpan(idsOrDrawables, glyphCount),
                               std::move(strikePromise.value())};
}

DrawableOpSubmitter::DrawableOpSubmitter(
        SkScalar strikeToSourceScale,
        SkSpan<SkPoint> positions,
        SkSpan<IDOrDrawable> idsOrDrawables,
        SkStrikePromise&& strikePromise)
        : fStrikeToSourceScale{strikeToSourceScale}
        , fPositions{positions}
        , fIDsOrDrawables{idsOrDrawables}
        , fStrikePromise(std::move(strikePromise)) {
    SkASSERT(!fPositions.empty());
}

void
DrawableOpSubmitter::submitDraws(SkCanvas* canvas, SkPoint drawOrigin,const SkPaint& paint) const {
    // Convert glyph IDs to Drawables if it hasn't been done yet.
    fConvertIDsToDrawables([&]() {
        fStrikePromise.strike()->glyphIDsToDrawables(fIDsOrDrawables);
        // Do not call resetStrike() because the strike must remain owned to ensure the Drawable
        // data is not freed.
    });

    // Calculate the matrix that maps the path glyphs from their size in the strike to
    // the graphics source space.
    SkMatrix strikeToSource = SkMatrix::Scale(fStrikeToSourceScale, fStrikeToSourceScale);
    strikeToSource.postTranslate(drawOrigin.x(), drawOrigin.y());

    // Transform the path to device because the deviceMatrix must be unchanged to
    // draw effect, filter or shader paths.
    for (auto [i, position] : SkMakeEnumerate(fPositions)) {
        SkDrawable* drawable = fIDsOrDrawables[i].fDrawable;

        if (drawable == nullptr) {
            // This better be pinned to keep the drawable data alive.
            fStrikePromise.strike()->verifyPinnedStrike();
            SkDEBUGFAIL("Drawable should not be nullptr.");
            continue;
        }

        // Transform the glyph to source space.
        SkMatrix pathMatrix = strikeToSource;
        pathMatrix.postTranslate(position.x(), position.y());

        SkAutoCanvasRestore acr(canvas, false);
        SkRect drawableBounds = drawable->getBounds();
        pathMatrix.mapRect(&drawableBounds);
        canvas->saveLayer(&drawableBounds, &paint);
        drawable->draw(canvas, &pathMatrix);
    }
}

// -- DrawableSubRun -------------------------------------------------------------------------------
class DrawableSubRun : public SubRun {
public:
    DrawableSubRun(DrawableOpSubmitter&& drawingDrawing)
            : fDrawingDrawing(std::move(drawingDrawing)) {}

    static SubRunOwner Make(SkZip<const SkGlyphID, const SkPoint> drawables,
                            SkScalar strikeToSourceScale,
                            SkStrikePromise&& strikePromise,
                            SubRunAllocator* alloc) {
        return alloc->makeUnique<DrawableSubRun>(
                DrawableOpSubmitter::Make(drawables,
                                          strikeToSourceScale,
                                          std::move(strikePromise),
                                          alloc));
    }

    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client);

    void draw(SkCanvas* canvas,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt>,
              const AtlasDrawDelegate&) const override {
        fDrawingDrawing.submitDraws(canvas, drawOrigin, paint);
    }

    int unflattenSize() const override;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override;

    const AtlasSubRun* testingOnly_atlasSubRun() const override;

protected:
    SubRunStreamTag subRunStreamTag() const override { return SubRunStreamTag::kDrawableStreamTag; }
    void doFlatten(SkWriteBuffer& buffer) const override;

private:
    DrawableOpSubmitter fDrawingDrawing;
};

int DrawableSubRun::unflattenSize() const {
    return sizeof(DrawableSubRun) + fDrawingDrawing.unflattenSize();
}

void DrawableSubRun::doFlatten(SkWriteBuffer& buffer) const {
    fDrawingDrawing.flatten(buffer);
}

SubRunOwner DrawableSubRun::MakeFromBuffer(SkReadBuffer& buffer,
                                           SubRunAllocator* alloc,
                                           const SkStrikeClient* client) {
    auto drawableOpSubmitter = DrawableOpSubmitter::MakeFromBuffer(buffer, alloc, client);
    if (!buffer.validate(drawableOpSubmitter.has_value())) { return nullptr; }
    return alloc->makeUnique<DrawableSubRun>(std::move(*drawableOpSubmitter));
}

bool DrawableSubRun::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    return true;
}

const AtlasSubRun* DrawableSubRun::testingOnly_atlasSubRun() const {
    return nullptr;
}

// -- DirectMaskSubRun -----------------------------------------------------------------------------
class DirectMaskSubRun final : public AtlasSubRun {
public:
    DirectMaskSubRun(VertexFiller&& vertexFiller,
                     GlyphVector&& glyphs)
            : AtlasSubRun(std::move(vertexFiller), std::move(glyphs)) {}

    static SubRunOwner Make(SkRect creationBounds,
                            SkZip<const SkPackedGlyphID, const SkPoint> accepted,
                            const SkMatrix& creationMatrix,
                            SkStrikePromise&& strikePromise,
                            MaskFormat maskType,
                            SubRunAllocator* alloc) {
        auto vertexFiller = VertexFiller::Make(maskType,
                                               creationMatrix,
                                               creationBounds,
                                               get_positions(accepted),
                                               alloc,
                                               kIsDirect);

        auto glyphVector =
                GlyphVector::Make(std::move(strikePromise), get_packedIDs(accepted), alloc);

        return alloc->makeUnique<DirectMaskSubRun>(std::move(vertexFiller), std::move(glyphVector));
    }

    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client) {
        auto vertexFiller = VertexFiller::MakeFromBuffer(buffer, alloc);
        if (!buffer.validate(vertexFiller.has_value())) { return nullptr; }

        auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
        if (!buffer.validate(glyphVector.has_value())) { return nullptr; }
        if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) {
            return nullptr;
        }

        SkASSERT(buffer.isValid());
        return alloc->makeUnique<DirectMaskSubRun>(
                std::move(*vertexFiller), std::move(*glyphVector));
    }

    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              const AtlasDrawDelegate& drawAtlas) const override {
        drawAtlas(this, drawOrigin, paint, std::move(subRunStorage),
                  {/* isSDF = */false, fVertexFiller.isLCD(), fVertexFiller.grMaskType()});
    }

    int unflattenSize() const override {
        return sizeof(DirectMaskSubRun) +
               fGlyphs.unflattenSize() +
               fVertexFiller.unflattenSize();
    }

    int glyphSrcPadding() const override { return 0; }

    void testingOnly_packedGlyphIDToGlyph(StrikeCache* cache) const override {
        fGlyphs.packedGlyphIDToGlyph(cache);
    }

    std::tuple<bool, SkRect> deviceRectAndNeedsTransform(
            const SkMatrix &positionMatrix) const override {
        auto [integerTranslate, deviceRect] =
                fVertexFiller.deviceRectAndCheckTransform(positionMatrix);
        return {!integerTranslate, deviceRect};
    }

    GlyphParams glyphParams() const override {
        // Since this is non-SDF, isAA will be ignored so we just pass true
        return { /*isSDF=*/false, fVertexFiller.isLCD(), /*isAA=*/true };
    }

    std::tuple<bool, int> regenerateAtlas(int begin, int end,
                                          RegenerateAtlasDelegate regenerateAtlas) const override {
        return regenerateAtlas(
                &fGlyphs, begin, end, fVertexFiller.grMaskType(), this->glyphSrcPadding());
    }

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        auto [reuse, _] = fVertexFiller.deviceRectAndCheckTransform(positionMatrix);
        return reuse;
    }

    const AtlasSubRun* testingOnly_atlasSubRun() const override {
        return this;
    }

protected:
    SubRunStreamTag subRunStreamTag() const override {
        return SubRunStreamTag::kDirectMaskStreamTag;
    }

    void doFlatten(SkWriteBuffer& buffer) const override {
        fVertexFiller.flatten(buffer);
        fGlyphs.flatten(buffer);
    }
};

// -- TransformedMaskSubRun ------------------------------------------------------------------------
class TransformedMaskSubRun final : public AtlasSubRun {
public:
    TransformedMaskSubRun(bool isBigEnough,
                          VertexFiller&& vertexFiller,
                          GlyphVector&& glyphs)
            : AtlasSubRun(std::move(vertexFiller), std::move(glyphs))
            , fIsBigEnough{isBigEnough} {}

    static SubRunOwner Make(SkZip<const SkPackedGlyphID, const SkPoint> accepted,
                            const SkMatrix& initialPositionMatrix,
                            SkStrikePromise&& strikePromise,
                            SkMatrix creationMatrix,
                            SkRect creationBounds,
                            MaskFormat maskType,
                            SubRunAllocator* alloc) {
        auto vertexFiller = VertexFiller::Make(maskType,
                                               creationMatrix,
                                               creationBounds,
                                               get_positions(accepted),
                                               alloc,
                                               kIsTransformed);

        auto glyphVector = GlyphVector::Make(
                std::move(strikePromise), get_packedIDs(accepted), alloc);

        return alloc->makeUnique<TransformedMaskSubRun>(
                initialPositionMatrix.getMaxScale() >= 1,
                std::move(vertexFiller),
                std::move(glyphVector));
    }

    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client) {
        auto vertexFiller = VertexFiller::MakeFromBuffer(buffer, alloc);
        if (!buffer.validate(vertexFiller.has_value())) { return nullptr; }

        auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
        if (!buffer.validate(glyphVector.has_value())) { return nullptr; }
        if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) {
            return nullptr;
        }
        const bool isBigEnough = buffer.readBool();
        return alloc->makeUnique<TransformedMaskSubRun>(
                isBigEnough, std::move(*vertexFiller), std::move(*glyphVector));
    }

    int unflattenSize() const override {
        return sizeof(TransformedMaskSubRun) +
               fGlyphs.unflattenSize() +
               fVertexFiller.unflattenSize();
    }

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        // If we are not scaling the cache entry to be larger, than a cache with smaller glyphs may
        // be better.
        return fIsBigEnough;
    }

    const AtlasSubRun* testingOnly_atlasSubRun() const override { return this; }

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override {
        fGlyphs.packedGlyphIDToGlyph(cache);
    }

    int glyphSrcPadding() const override { return 1; }

    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              const AtlasDrawDelegate& drawAtlas) const override {
        drawAtlas(this, drawOrigin, paint, std::move(subRunStorage),
                  {/* isSDF = */false, fVertexFiller.isLCD(), fVertexFiller.grMaskType()});
    }

    std::tuple<bool, SkRect> deviceRectAndNeedsTransform(
            const SkMatrix &positionMatrix) const override {
        auto [_, deviceRect] = fVertexFiller.deviceRectAndCheckTransform(positionMatrix);
        return {true, deviceRect};
    }

    GlyphParams glyphParams() const override {
        // Since this is non-SDF, isAA will be ignored so we just pass true
        return { /*isSDF=*/false, fVertexFiller.isLCD(), /*isAA=*/true };
    }

    std::tuple<bool, int> regenerateAtlas(int begin, int end,
                                          RegenerateAtlasDelegate regenerateAtlas) const override {
        return regenerateAtlas(
                &fGlyphs, begin, end, fVertexFiller.grMaskType(), this->glyphSrcPadding());
    }

protected:
    SubRunStreamTag subRunStreamTag() const override {
        return SubRunStreamTag::kTransformMaskStreamTag;
    }

    void doFlatten(SkWriteBuffer& buffer) const override {
        fVertexFiller.flatten(buffer);
        fGlyphs.flatten(buffer);
        buffer.writeBool(fIsBigEnough);
    }

private:
    const bool fIsBigEnough;
};  // class TransformedMaskSubRun

// -- SDFTSubRun -----------------------------------------------------------------------------------

bool has_some_antialiasing(const SkFont& font ) {
    SkFont::Edging edging = font.getEdging();
    return edging == SkFont::Edging::kAntiAlias
           || edging == SkFont::Edging::kSubpixelAntiAlias;
}

#if !defined(SK_DISABLE_SDF_TEXT)

class SDFTSubRun final : public AtlasSubRun {
public:
    SDFTSubRun(bool useLCDText,
               bool antiAliased,
               const SDFTMatrixRange& matrixRange,
               VertexFiller&& vertexFiller,
               GlyphVector&& glyphs)
            : AtlasSubRun(std::move(vertexFiller), std::move(glyphs))
            , fUseLCDText{useLCDText}
            , fAntiAliased{antiAliased}
            , fMatrixRange{matrixRange} {
        SkASSERT(fVertexFiller.grMaskType() == MaskFormat::kA8);
    }

    static SubRunOwner Make(SkZip<const SkPackedGlyphID, const SkPoint> accepted,
                            const SkFont& runFont,
                            SkStrikePromise&& strikePromise,
                            const SkMatrix& creationMatrix,
                            SkRect creationBounds,
                            const SDFTMatrixRange& matrixRange,
                            SubRunAllocator* alloc) {
        auto vertexFiller = VertexFiller::Make(MaskFormat::kA8,
                                               creationMatrix,
                                               creationBounds,
                                               get_positions(accepted),
                                               alloc,
                                               kIsTransformed);

        auto glyphVector = GlyphVector::Make(
                std::move(strikePromise), get_packedIDs(accepted), alloc);

        return alloc->makeUnique<SDFTSubRun>(
                runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
                has_some_antialiasing(runFont),
                matrixRange,
                std::move(vertexFiller),
                std::move(glyphVector));
    }

    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
                                      SubRunAllocator* alloc,
                                      const SkStrikeClient* client) {
        int useLCD = buffer.readInt();
        int isAntiAliased = buffer.readInt();
        SDFTMatrixRange matrixRange = SDFTMatrixRange::MakeFromBuffer(buffer);
        auto vertexFiller = VertexFiller::MakeFromBuffer(buffer, alloc);
        if (!buffer.validate(vertexFiller.has_value())) { return nullptr; }
        if (!buffer.validate(vertexFiller.value().grMaskType() == MaskFormat::kA8)) {
            return nullptr;
        }
        auto glyphVector = GlyphVector::MakeFromBuffer(buffer, client, alloc);
        if (!buffer.validate(glyphVector.has_value())) { return nullptr; }
        if (!buffer.validate(SkCount(glyphVector->glyphs()) == vertexFiller->count())) {
            return nullptr;
        }
        return alloc->makeUnique<SDFTSubRun>(useLCD,
                                             isAntiAliased,
                                             matrixRange,
                                             std::move(*vertexFiller),
                                             std::move(*glyphVector));
    }

    int unflattenSize() const override {
        return sizeof(SDFTSubRun) + fGlyphs.unflattenSize() + fVertexFiller.unflattenSize();
    }

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const override {
        return fMatrixRange.matrixInRange(positionMatrix);
    }

    const AtlasSubRun* testingOnly_atlasSubRun() const override { return this; }

    void testingOnly_packedGlyphIDToGlyph(StrikeCache *cache) const override {
        fGlyphs.packedGlyphIDToGlyph(cache);
    }

    int glyphSrcPadding() const override { return SK_DistanceFieldInset; }

    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint& paint,
              sk_sp<SkRefCnt> subRunStorage,
              const AtlasDrawDelegate& drawAtlas) const override {
        drawAtlas(this, drawOrigin, paint, std::move(subRunStorage),
                  {/* isSDF = */true, /* isLCD = */fUseLCDText, skgpu::MaskFormat::kA8});
    }

    std::tuple<bool, SkRect> deviceRectAndNeedsTransform(
            const SkMatrix &positionMatrix) const override {
        auto [_, deviceRect] = fVertexFiller.deviceRectAndCheckTransform(positionMatrix);
        return {true, deviceRect};
    }

    GlyphParams glyphParams() const override {
        return { /*isSDF=*/true, fUseLCDText, /*isAA=*/fAntiAliased };
    }

    std::tuple<bool, int> regenerateAtlas(int begin, int end,
                                          RegenerateAtlasDelegate regenerateAtlas) const override {
        return regenerateAtlas(&fGlyphs, begin, end, MaskFormat::kA8, this->glyphSrcPadding());
    }

protected:
    SubRunStreamTag subRunStreamTag() const override { return SubRunStreamTag::kSDFTStreamTag; }
    void doFlatten(SkWriteBuffer& buffer) const override {
        buffer.writeInt(fUseLCDText);
        buffer.writeInt(fAntiAliased);
        fMatrixRange.flatten(buffer);
        fVertexFiller.flatten(buffer);
        fGlyphs.flatten(buffer);
    }

private:
    const bool fUseLCDText;
    const bool fAntiAliased;
    const SDFTMatrixRange fMatrixRange;
};  // class SDFTSubRun

#endif // !defined(SK_DISABLE_SDF_TEXT)

// -- SubRun ---------------------------------------------------------------------------------------

template<typename AddSingleMaskFormat>
void add_multi_mask_format(
        AddSingleMaskFormat addSingleMaskFormat,
        SkZip<const SkPackedGlyphID, const SkPoint, const SkMask::Format> accepted) {
    if (accepted.empty()) { return; }

    auto maskSpan = accepted.get<2>();
    MaskFormat format = Glyph::FormatFromSkGlyph(maskSpan[0]);
    size_t startIndex = 0;
    for (size_t i = 1; i < accepted.size(); i++) {
        MaskFormat nextFormat = Glyph::FormatFromSkGlyph(maskSpan[i]);
        if (format != nextFormat) {
            auto interval = accepted.subspan(startIndex, i - startIndex);
            // Only pass the packed glyph ids and positions.
            auto glyphsWithSameFormat = SkMakeZip(interval.get<0>(), interval.get<1>());
            // Take a ref on the strike. This should rarely happen.
            addSingleMaskFormat(glyphsWithSameFormat, format);
            format = nextFormat;
            startIndex = i;
        }
    }
    auto interval = accepted.last(accepted.size() - startIndex);
    auto glyphsWithSameFormat = SkMakeZip(interval.get<0>(), interval.get<1>());
    addSingleMaskFormat(glyphsWithSameFormat, format);
}
}  // anonymous namespace

namespace sktext::gpu {
SubRun::~SubRun() = default;
void SubRun::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt(this->subRunStreamTag());
    this->doFlatten(buffer);
}

SubRunOwner SubRun::MakeFromBuffer(SkReadBuffer& buffer,
                                   SubRunAllocator* alloc,
                                   const SkStrikeClient* client) {
    using Maker = SubRunOwner (*)(SkReadBuffer&,
                                  SubRunAllocator*,
                                  const SkStrikeClient*);

    static Maker makers[kSubRunStreamTagCount] = {
            nullptr,                                             // 0 index is bad.
            DirectMaskSubRun::MakeFromBuffer,
#if !defined(SK_DISABLE_SDF_TEXT)
            SDFTSubRun::MakeFromBuffer,
#endif
            TransformedMaskSubRun::MakeFromBuffer,
            PathSubRun::MakeFromBuffer,
            DrawableSubRun::MakeFromBuffer,
    };
    int subRunTypeInt = buffer.readInt();
    if (!buffer.validate(kBad < subRunTypeInt && subRunTypeInt < kSubRunStreamTagCount)) {
        return nullptr;
    }
    auto maker = makers[subRunTypeInt];
    if (!buffer.validate(maker != nullptr)) { return nullptr; }
    return maker(buffer, alloc, client);
}

// -- SubRunContainer ------------------------------------------------------------------------------
SubRunContainer::SubRunContainer(const SkMatrix& initialPositionMatrix)
        : fInitialPositionMatrix{initialPositionMatrix} {}

void SubRunContainer::flattenAllocSizeHint(SkWriteBuffer& buffer) const {
    int unflattenSizeHint = 0;
    for (auto& subrun : fSubRuns) {
        unflattenSizeHint += subrun.unflattenSize();
    }
    buffer.writeInt(unflattenSizeHint);
}

int SubRunContainer::AllocSizeHintFromBuffer(SkReadBuffer& buffer) {
    int subRunsSizeHint = buffer.readInt();

    // Since the hint doesn't affect correctness, if it looks fishy just pick a reasonable
    // value.
    if (subRunsSizeHint < 0 || (1 << 16) < subRunsSizeHint) {
        subRunsSizeHint = 128;
    }
    return subRunsSizeHint;
}

void SubRunContainer::flattenRuns(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(fInitialPositionMatrix);
    int subRunCount = 0;
    for ([[maybe_unused]] auto& subRun : fSubRuns) {
        subRunCount += 1;
    }
    buffer.writeInt(subRunCount);
    for (auto& subRun : fSubRuns) {
        subRun.flatten(buffer);
    }
}

SubRunContainerOwner SubRunContainer::MakeFromBufferInAlloc(SkReadBuffer& buffer,
                                                            const SkStrikeClient* client,
                                                            SubRunAllocator* alloc) {
    SkMatrix positionMatrix;
    buffer.readMatrix(&positionMatrix);
    if (!buffer.isValid()) { return nullptr; }
    SubRunContainerOwner container = alloc->makeUnique<SubRunContainer>(positionMatrix);

    int subRunCount = buffer.readInt();
    if (!buffer.validate(subRunCount > 0)) { return nullptr; }
    for (int i = 0; i < subRunCount; ++i) {
        auto subRunOwner = SubRun::MakeFromBuffer(buffer, alloc, client);
        if (!buffer.validate(subRunOwner != nullptr)) { return nullptr; }
        if (subRunOwner != nullptr) {
            container->fSubRuns.append(std::move(subRunOwner));
        }
    }
    return container;
}

size_t SubRunContainer::EstimateAllocSize(const GlyphRunList& glyphRunList) {
    // The difference in alignment from the per-glyph data to the SubRun;
    constexpr size_t alignDiff = alignof(DirectMaskSubRun) - alignof(SkPoint);
    constexpr size_t vertexDataToSubRunPadding = alignDiff > 0 ? alignDiff : 0;
    size_t totalGlyphCount = glyphRunList.totalGlyphCount();
    // This is optimized for DirectMaskSubRun which is by far the most common case.
    return totalGlyphCount * sizeof(SkPoint)
           + GlyphVector::GlyphVectorSize(totalGlyphCount)
           + glyphRunList.runCount() * (sizeof(DirectMaskSubRun) + vertexDataToSubRunPadding)
           + sizeof(SubRunContainer);
}

SkScalar find_maximum_glyph_dimension(StrikeForGPU* strike, SkSpan<const SkGlyphID> glyphs) {
    StrikeMutationMonitor m{strike};
    SkScalar maxDimension = 0;
    for (SkGlyphID glyphID : glyphs) {
        SkGlyphDigest digest = strike->digestFor(kMask, SkPackedGlyphID{glyphID});
        maxDimension = std::max(static_cast<SkScalar>(digest.maxDimension()), maxDimension);
    }

    return maxDimension;
}

#if !defined(SK_DISABLE_SDF_TEXT)
std::tuple<SkZip<const SkPackedGlyphID, const SkPoint>, SkZip<SkGlyphID, SkPoint>, SkRect>
prepare_for_SDFT_drawing(StrikeForGPU* strike,
                         const SkMatrix& creationMatrix,
                         SkZip<const SkGlyphID, const SkPoint> source,
                         SkZip<SkPackedGlyphID, SkPoint> acceptedBuffer,
                         SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0,
        rejectedSize = 0;
    SkGlyphRect boundingRect = skglyph::empty_rect();
    StrikeMutationMonitor m{strike};
    for (const auto [glyphID, pos] : source) {
        if (!SkIsFinite(pos.x(), pos.y())) {
            continue;
        }

        const SkPackedGlyphID packedID{glyphID};
        switch (const SkGlyphDigest digest = strike->digestFor(skglyph::kSDFT, packedID);
                digest.actionFor(skglyph::kSDFT)) {
            case GlyphAction::kAccept: {
                SkPoint mappedPos = creationMatrix.mapPoint(pos);
                const SkGlyphRect glyphBounds =
                    digest.bounds()
                        // The SDFT glyphs have 2-pixel wide padding that should
                        // not be used in calculating the source rectangle.
                        .inset(SK_DistanceFieldInset, SK_DistanceFieldInset)
                        .offset(mappedPos);
                boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                acceptedBuffer[acceptedSize++] = std::make_tuple(packedID, glyphBounds.leftTop());
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
            break;
            default:
                break;
        }
    }

    return {acceptedBuffer.first(acceptedSize),
            rejectedBuffer.first(rejectedSize),
            boundingRect.rect()};
}
#endif

std::tuple<SkZip<const SkPackedGlyphID, const SkPoint, const SkMask::Format>,
           SkZip<SkGlyphID, SkPoint>,
           SkRect>
prepare_for_direct_mask_drawing(StrikeForGPU* strike,
                                const SkMatrix& positionMatrix,
                                SkZip<const SkGlyphID, const SkPoint> source,
                                SkZip<SkPackedGlyphID, SkPoint, SkMask::Format> acceptedBuffer,
                                SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    const SkIPoint mask = strike->roundingSpec().ignorePositionFieldMask;
    const SkPoint halfSampleFreq = strike->roundingSpec().halfAxisSampleFreq;

    // Build up the mapping from source space to device space. Add the rounding constant
    // halfSampleFreq, so we just need to floor to get the device result.
    SkMatrix positionMatrixWithRounding = positionMatrix;
    positionMatrixWithRounding.postTranslate(halfSampleFreq.x(), halfSampleFreq.y());

    int acceptedSize = 0,
        rejectedSize = 0;
    SkGlyphRect boundingRect = skglyph::empty_rect();
    StrikeMutationMonitor m{strike};
    for (auto [glyphID, pos] : source) {
        if (!SkIsFinite(pos.x(), pos.y())) {
            continue;
        }

        const SkPoint mappedPos = positionMatrixWithRounding.mapPoint(pos);
        const SkPackedGlyphID packedID{glyphID, mappedPos, mask};
        switch (const SkGlyphDigest digest = strike->digestFor(skglyph::kDirectMask, packedID);
                digest.actionFor(skglyph::kDirectMask)) {
            case GlyphAction::kAccept: {
                const SkPoint roundedPos{SkScalarFloorToScalar(mappedPos.x()),
                                         SkScalarFloorToScalar(mappedPos.y())};
                const SkGlyphRect glyphBounds = digest.bounds().offset(roundedPos);
                boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                acceptedBuffer[acceptedSize++] =
                        std::make_tuple(packedID, glyphBounds.leftTop(), digest.maskFormat());
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }

    return {acceptedBuffer.first(acceptedSize),
            rejectedBuffer.first(rejectedSize),
            boundingRect.rect()};
}

std::tuple<SkZip<const SkPackedGlyphID, const SkPoint, const SkMask::Format>,
           SkZip<SkGlyphID, SkPoint>,
           SkRect>
prepare_for_mask_drawing(StrikeForGPU* strike,
                         const SkMatrix& creationMatrix,
                         SkZip<const SkGlyphID, const SkPoint> source,
                         SkZip<SkPackedGlyphID, SkPoint, SkMask::Format> acceptedBuffer,
                         SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0,
        rejectedSize = 0;
    SkGlyphRect boundingRect = skglyph::empty_rect();
    StrikeMutationMonitor m{strike};
    for (auto [glyphID, pos] : source) {
        if (!SkIsFinite(pos.x(), pos.y())) {
            continue;
        }

        const SkPackedGlyphID packedID{glyphID};
        switch (const SkGlyphDigest digest = strike->digestFor(kMask, packedID);
                digest.actionFor(kMask)) {
            case GlyphAction::kAccept: {
                const SkPoint mappedPos = creationMatrix.mapPoint(pos);
                const SkGlyphRect glyphBounds = digest.bounds().offset(mappedPos);
                boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                acceptedBuffer[acceptedSize++] =
                        std::make_tuple(packedID, glyphBounds.leftTop(), digest.maskFormat());
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }

    return {acceptedBuffer.first(acceptedSize),
            rejectedBuffer.first(rejectedSize),
            boundingRect.rect()};
}

std::tuple<SkZip<const SkGlyphID, const SkPoint>, SkZip<SkGlyphID, SkPoint>>
prepare_for_path_drawing(StrikeForGPU* strike,
                         SkZip<const SkGlyphID, const SkPoint> source,
                         SkZip<SkGlyphID, SkPoint> acceptedBuffer,
                         SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0;
    int rejectedSize = 0;
    StrikeMutationMonitor m{strike};
    for (const auto [glyphID, pos] : source) {
        if (!SkIsFinite(pos.x(), pos.y())) {
            continue;
        }

        switch (strike->digestFor(skglyph::kPath, SkPackedGlyphID{glyphID})
                       .actionFor(skglyph::kPath)) {
            case GlyphAction::kAccept:
                acceptedBuffer[acceptedSize++] = std::make_tuple(glyphID, pos);
                break;
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }
    return {acceptedBuffer.first(acceptedSize), rejectedBuffer.first(rejectedSize)};
}

std::tuple<SkZip<const SkGlyphID, const SkPoint>, SkZip<SkGlyphID, SkPoint>>
 prepare_for_drawable_drawing(StrikeForGPU* strike,
                             SkZip<const SkGlyphID, const SkPoint> source,
                             SkZip<SkGlyphID, SkPoint> acceptedBuffer,
                             SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    int acceptedSize = 0;
    int rejectedSize = 0;
    StrikeMutationMonitor m{strike};
    for (const auto [glyphID, pos] : source) {
        if (!SkIsFinite(pos.x(), pos.y())) {
            continue;
        }

        switch (strike->digestFor(skglyph::kDrawable, SkPackedGlyphID{glyphID})
                       .actionFor(skglyph::kDrawable)) {
            case GlyphAction::kAccept:
                acceptedBuffer[acceptedSize++] = std::make_tuple(glyphID, pos);
                break;
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }
    return {acceptedBuffer.first(acceptedSize), rejectedBuffer.first(rejectedSize)};
}

#if !defined(SK_DISABLE_SDF_TEXT)
static std::tuple<SkStrikeSpec, SkScalar, sktext::gpu::SDFTMatrixRange>
make_sdft_strike_spec(const SkFont& font, const SkPaint& paint,
                      const SkSurfaceProps& surfaceProps, const SkMatrix& deviceMatrix,
                      const SkPoint& textLocation, const sktext::gpu::SubRunControl& control) {
    // Add filter to the paint which creates the SDFT data for A8 masks.
    SkPaint dfPaint{paint};
    dfPaint.setMaskFilter(sktext::gpu::SDFMaskFilter::Make());

    auto [dfFont, strikeToSourceScale, matrixRange] = control.getSDFFont(font, deviceMatrix,
                                                                         textLocation);

    // Adjust the stroke width by the scale factor for drawing the SDFT.
    dfPaint.setStrokeWidth(paint.getStrokeWidth() / strikeToSourceScale);

    // Check for dashing and adjust the intervals.
    if (SkPathEffect* pathEffect = paint.getPathEffect(); pathEffect != nullptr) {
        if (auto info = as_PEB(pathEffect)->asADash()) {
            SkSpan<const SkScalar> src = info->fIntervals;
            SkASSERT(src.size() > 1);
            // Allocate the intervals.
            std::vector<SkScalar> scaledIntervals(src.size());
            for (size_t i = 0; i < src.size(); ++i) {
                scaledIntervals[i] = src[i] / strikeToSourceScale;
            }
            auto scaledDashes = SkDashPathEffect::Make(scaledIntervals,
                                                       info->fPhase / strikeToSourceScale);
            dfPaint.setPathEffect(scaledDashes);
        }
    }

    // Fake-gamma and subpixel antialiasing are applied in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkScalerContextFlags flags = SkScalerContextFlags::kNone;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(dfFont, dfPaint, surfaceProps, flags,
                                                     SkMatrix::I());

    return std::make_tuple(std::move(strikeSpec), strikeToSourceScale, matrixRange);
}
#endif

SubRunContainerOwner SubRunContainer::MakeInAlloc(
        const GlyphRunList& glyphRunList,
        const SkMatrix& positionMatrix,
        const SkPaint& runPaint,
        SkStrikeDeviceInfo strikeDeviceInfo,
        StrikeForGPUCacheInterface* strikeCache,
        SubRunAllocator* alloc,
        SubRunCreationBehavior creationBehavior,
        const char* tag) {
    SkASSERT(alloc != nullptr);
    SkASSERT(strikeDeviceInfo.fSubRunControl != nullptr);

    SubRunContainerOwner container = alloc->makeUnique<SubRunContainer>(positionMatrix);
    // If there is no SubRunControl description ignore all SubRuns.
    if (strikeDeviceInfo.fSubRunControl == nullptr) {
        return container;
    }

    const SkSurfaceProps deviceProps = strikeDeviceInfo.fSurfaceProps;
    const SkScalerContextFlags scalerContextFlags = strikeDeviceInfo.fScalerContextFlags;
    const SubRunControl* subRunControl = strikeDeviceInfo.fSubRunControl;
#if !defined(SK_DISABLE_SDF_TEXT)
    const SkScalar maxMaskSize = subRunControl->maxSize();
#else
    const SkScalar maxMaskSize = 256;
#endif

    // TODO: hoist the buffer structure to the GlyphRunBuilder. The buffer structure here is
    //  still begin tuned, and this is expected to be slower until tuned.
    const int maxGlyphRunSize = glyphRunList.maxGlyphRunSize();

    // Accepted buffers.
    STArray<64, SkPackedGlyphID> acceptedPackedGlyphIDs;
    STArray<64, SkGlyphID> acceptedGlyphIDs;
    STArray<64, SkPoint> acceptedPositions;
    STArray<64, SkMask::Format> acceptedFormats;
    acceptedPackedGlyphIDs.resize(maxGlyphRunSize);
    acceptedGlyphIDs.resize(maxGlyphRunSize);
    acceptedPositions.resize(maxGlyphRunSize);
    acceptedFormats.resize(maxGlyphRunSize);

    // Rejected buffers.
    STArray<64, SkGlyphID> rejectedGlyphIDs;
    STArray<64, SkPoint> rejectedPositions;
    rejectedGlyphIDs.resize(maxGlyphRunSize);
    rejectedPositions.resize(maxGlyphRunSize);
    const auto rejectedBuffer = SkMakeZip(rejectedGlyphIDs, rejectedPositions);

    const SkPoint glyphRunListLocation = glyphRunList.sourceBounds().center();

    // Handle all the runs in the glyphRunList
    for (auto& glyphRun : glyphRunList) {
        SkZip<const SkGlyphID, const SkPoint> source = glyphRun.source();
        const SkFont& runFont = glyphRun.font();

        const SkScalar approximateDeviceTextSize =
                // Since the positionMatrix has the origin prepended, use the plain
                // sourceBounds from above.
                SkFontPriv::ApproximateTransformedTextSize(runFont, positionMatrix,
                                                           glyphRunListLocation);

        // Atlas mask cases - SDFT and direct mask
        // Only consider using direct or SDFT drawing if not drawing hairlines and not too big.
        if ((runPaint.getStyle() != SkPaint::kStroke_Style || runPaint.getStrokeWidth() != 0) &&
                approximateDeviceTextSize < maxMaskSize) {

#if !defined(SK_DISABLE_SDF_TEXT)
            // SDFT case
            if (subRunControl->isSDFT(approximateDeviceTextSize, runPaint, positionMatrix)) {
                // Process SDFT - This should be the .009% case.
                const auto& [strikeSpec, strikeToSourceScale, matrixRange] =
                        make_sdft_strike_spec(
                                runFont, runPaint, deviceProps, positionMatrix,
                                glyphRunListLocation, *subRunControl);

                if (!SkScalarNearlyZero(strikeToSourceScale)) {
                    sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                    // The creationMatrix needs to scale the strike data when inverted and
                    // multiplied by the positionMatrix. The final CTM should be:
                    //   [positionMatrix][scale by strikeToSourceScale],
                    // which should equal the following because of the transform during the vertex
                    // calculation,
                    //   [positionMatrix][creationMatrix]^-1.
                    // So, the creation matrix needs to be
                    //   [scale by 1/strikeToSourceScale].
                    SkMatrix creationMatrix =
                            SkMatrix::Scale(1.f/strikeToSourceScale, 1.f/strikeToSourceScale);

                    auto acceptedBuffer = SkMakeZip(acceptedPackedGlyphIDs, acceptedPositions);
                    auto [accepted, rejected, creationBounds] = prepare_for_SDFT_drawing(
                            strike.get(), creationMatrix, source, acceptedBuffer, rejectedBuffer);
                    source = rejected;

                    if (creationBehavior == kAddSubRuns && !accepted.empty()) {
                        container->fSubRuns.append(SDFTSubRun::Make(
                                accepted,
                                runFont,
                                strike->strikePromise(),
                                creationMatrix,
                                creationBounds,
                                matrixRange,
                                alloc));
                    }
                }
            }
#endif  // !defined(SK_DISABLE_SDF_TEXT)
            // Mask filters with 3D format (e.g. EmbossMaskFilter) need to be drawn as a path
            // in order to apply the filter through the Canvas AutoLayer system.
            const bool needsAutoLayer = runPaint.getMaskFilter() &&
                as_MFB(runPaint.getMaskFilter())->getFormat() == SkMask::k3D_Format;
            // Direct Mask case
            // Handle all the directly mapped mask subruns.
            if (!source.empty() && !positionMatrix.hasPerspective() && !needsAutoLayer) {
                // Process masks including ARGB - this should be the 99.99% case.
                // This will handle medium size emoji that are sharing the run with SDFT drawn text.
                // If things are too big they will be passed along to the drawing of last resort
                // below.
                SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                        runFont, runPaint, deviceProps, scalerContextFlags, positionMatrix);

                sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                auto acceptedBuffer = SkMakeZip(acceptedPackedGlyphIDs,
                                                acceptedPositions,
                                                acceptedFormats);
                auto [accepted, rejected, creationBounds] = prepare_for_direct_mask_drawing(
                        strike.get(), positionMatrix, source, acceptedBuffer, rejectedBuffer);
                source = rejected;

                if (creationBehavior == kAddSubRuns && !accepted.empty()) {
                    auto addGlyphsWithSameFormat =
                        [&, bounds = creationBounds](
                                SkZip<const SkPackedGlyphID, const SkPoint> subrun,
                                MaskFormat format) {
                            container->fSubRuns.append(
                                    DirectMaskSubRun::Make(bounds,
                                                           subrun,
                                                           container->initialPosition(),
                                                           strike->strikePromise(),
                                                           format,
                                                           alloc));
                        };
                    add_multi_mask_format(addGlyphsWithSameFormat, accepted);
                }
            }
        }

        // Drawable case
        // Handle all the drawable glyphs - usually large or perspective color glyphs.
        if (!source.empty()) {
            auto [strikeSpec, strikeToSourceScale] =
                    SkStrikeSpec::MakePath(runFont, runPaint, deviceProps, scalerContextFlags);

            if (!SkScalarNearlyZero(strikeToSourceScale)) {
                sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                auto acceptedBuffer = SkMakeZip(acceptedGlyphIDs, acceptedPositions);
                auto [accepted, rejected] =
                prepare_for_drawable_drawing(strike.get(), source, acceptedBuffer, rejectedBuffer);
                source = rejected;

                if (creationBehavior == kAddSubRuns && !accepted.empty()) {
                    container->fSubRuns.append(
                            DrawableSubRun::Make(
                                accepted,
                                strikeToSourceScale,
                                strike->strikePromise(),
                                alloc));
                }
            }
        }

        // Path case
        // Handle path subruns. Mainly, large or large perspective glyphs with no color.
        if (!source.empty()) {
            auto [strikeSpec, strikeToSourceScale] =
                    SkStrikeSpec::MakePath(runFont, runPaint, deviceProps, scalerContextFlags);

            if (!SkScalarNearlyZero(strikeToSourceScale)) {
                sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

                auto acceptedBuffer = SkMakeZip(acceptedGlyphIDs, acceptedPositions);
                auto [accepted, rejected] =
                prepare_for_path_drawing(strike.get(), source, acceptedBuffer, rejectedBuffer);
                source = rejected;

                if (creationBehavior == kAddSubRuns && !accepted.empty()) {
                    const bool isAntiAliased =
                            subRunControl->forcePathAA() || has_some_antialiasing(runFont);
                    container->fSubRuns.append(
                            PathSubRun::Make(accepted,
                                             isAntiAliased,
                                             strikeToSourceScale,
                                             strike->strikePromise(),
                                             alloc));
                }
            }
        }

        // Drawing of last resort case
        // Draw all the rest of the rejected glyphs from above. This scales out of the atlas to
        // the screen, so quality will suffer. This mainly handles large color or perspective
        // color not handled by Drawables.
        if (!source.empty() && !SkScalarNearlyZero(approximateDeviceTextSize)) {
            // Creation matrix will be changed below to meet the following criteria:
            // * No perspective - the font scaler and the strikes can't handle perspective masks.
            // * Fits atlas - creationMatrix will be conditioned so that the maximum glyph
            //   dimension for this run will be <  kMaxBilerpAtlasDimension.
            SkMatrix creationMatrix = positionMatrix;

            // Condition creationMatrix for perspective.
            if (creationMatrix.hasPerspective()) {
                // Find a scale factor that reduces pixelation caused by keystoning.
                SkPoint center = glyphRunList.sourceBounds().center();
                SkScalar maxAreaScale = SkMatrixPriv::DifferentialAreaScale(creationMatrix, center);
                SkScalar perspectiveFactor = 1;
                if (SkIsFinite(maxAreaScale) && !SkScalarNearlyZero(maxAreaScale)) {
                    perspectiveFactor = SkScalarSqrt(maxAreaScale);
                }

                // Masks can not be created in perspective. Create a non-perspective font with a
                // scale that will support the perspective keystoning.
                creationMatrix = SkMatrix::Scale(perspectiveFactor, perspectiveFactor);
            }

            // Reduce to make a one pixel border for the bilerp padding.
            static const constexpr SkScalar kMaxBilerpAtlasDimension =
                    SkGlyphDigest::kSkSideTooBigForAtlas - 2;

            // Get the raw glyph IDs to simulate device drawing to figure the maximum device
            // dimension.
            const SkSpan<const SkGlyphID> glyphs = get_glyphIDs(source);

            // maxGlyphDimension always returns an integer even though the return type is SkScalar.
            auto maxGlyphDimension = [&](const SkMatrix& m) {
                const SkStrikeSpec strikeSpec = SkStrikeSpec::MakeTransformMask(
                        runFont, runPaint, deviceProps, scalerContextFlags, m);
                const sk_sp<StrikeForGPU> gaugingStrike =
                        strikeSpec.findOrCreateScopedStrike(strikeCache);
                const SkScalar maxDimension =
                        find_maximum_glyph_dimension(gaugingStrike.get(), glyphs);
                // TODO: There is a problem where a small character (say .) and a large
                //  character (say M) are in the same run. If the run is scaled to be very
                //  large, then the M may return 0 because its dimensions are > 65535, but
                //  the small character produces regular result because its largest dimension
                //  is < 65535. This will create an improper scale factor causing the M to be
                //  too large to fit in the atlas. Tracked by skbug.com/40044801.
                return maxDimension;
            };

            // Condition the creationMatrix so that glyphs fit in the atlas.
            for (SkScalar maxDimension = maxGlyphDimension(creationMatrix);
                 kMaxBilerpAtlasDimension < maxDimension;
                 maxDimension = maxGlyphDimension(creationMatrix))
            {
                // The SkScalerContext has a limit of 65536 maximum dimension.
                // reductionFactor will always be < 1 because
                // maxDimension > kMaxBilerpAtlasDimension, and because maxDimension will always
                // be an integer the reduction factor will always be at most 254 / 255.
                SkScalar reductionFactor = kMaxBilerpAtlasDimension / maxDimension;
                creationMatrix.postScale(reductionFactor, reductionFactor);
            }

            // Draw using the creationMatrix.
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeTransformMask(
                    runFont, runPaint, deviceProps, scalerContextFlags, creationMatrix);

            sk_sp<StrikeForGPU> strike = strikeSpec.findOrCreateScopedStrike(strikeCache);

            auto acceptedBuffer =
                    SkMakeZip(acceptedPackedGlyphIDs, acceptedPositions, acceptedFormats);
            auto [accepted, rejected, creationBounds] =
                prepare_for_mask_drawing(
                        strike.get(), creationMatrix, source, acceptedBuffer, rejectedBuffer);
            source = rejected;

            if (creationBehavior == kAddSubRuns && !accepted.empty()) {

                auto addGlyphsWithSameFormat =
                        [&, bounds = creationBounds](
                                SkZip<const SkPackedGlyphID, const SkPoint> subrun,
                                MaskFormat format) {
                            container->fSubRuns.append(
                                    TransformedMaskSubRun::Make(subrun,
                                                                container->initialPosition(),
                                                                strike->strikePromise(),
                                                                creationMatrix,
                                                                bounds,
                                                                format,
                                                                alloc));
                        };
                add_multi_mask_format(addGlyphsWithSameFormat, accepted);
            }
        }
    }

    return container;
}

void SubRunContainer::draw(SkCanvas* canvas,
                           SkPoint drawOrigin,
                           const SkPaint& paint,
                           const SkRefCnt* subRunStorage,
                           const AtlasDrawDelegate& atlasDelegate) const {
    for (auto& subRun : fSubRuns) {
        subRun.draw(canvas, drawOrigin, paint, sk_ref_sp(subRunStorage), atlasDelegate);
    }
}

bool SubRunContainer::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    for (const SubRun& subRun : fSubRuns) {
        if (!subRun.canReuse(paint, positionMatrix)) {
            return false;
        }
    }
    return true;
}

// Returns the empty span if there is a problem reading the positions.
SkSpan<SkPoint> MakePointsFromBuffer(SkReadBuffer& buffer, SubRunAllocator* alloc) {
    uint32_t glyphCount = buffer.getArrayCount();

    // Zero indicates a problem with serialization.
    if (!buffer.validate(glyphCount != 0)) { return {}; }

    // Check that the count will not overflow the arena.
    if (!buffer.validate(glyphCount <= INT_MAX &&
                         BagOfBytes::WillCountFit<SkPoint>(glyphCount))) { return {}; }

    SkPoint* positionsData = alloc->makePODArray<SkPoint>(glyphCount);
    if (!buffer.readPointArray({positionsData, glyphCount})) { return {}; }
    return {positionsData, glyphCount};
}

}  // namespace sktext::gpu
