/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/TextBlob.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

#if SK_SUPPORT_GPU  // Ganesh Support
#include "src/gpu/ganesh/Device_v1.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#endif

using namespace sktext::gpu;
namespace {
SkMatrix position_matrix(const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    SkMatrix position_matrix = drawMatrix;
    return position_matrix.preTranslate(drawOrigin.x(), drawOrigin.y());
}

// Check for integer translate with the same 2x2 matrix.
// Returns the translation, and true if the change from initial matrix to the position matrix
// support using direct glyph masks.
std::tuple<bool, SkVector> can_use_direct(
        const SkMatrix& initialPositionMatrix, const SkMatrix& positionMatrix) {
    // The existing direct glyph info can be used if the initialPositionMatrix, and the
    // positionMatrix have the same 2x2, and the translation between them is integer.
    // Calculate the translation in source space to a translation in device space by mapping
    // (0, 0) through both the initial position matrix and the position matrix; take the difference.
    SkVector translation = positionMatrix.mapOrigin() - initialPositionMatrix.mapOrigin();
    return {initialPositionMatrix.getScaleX() == positionMatrix.getScaleX() &&
            initialPositionMatrix.getScaleY() == positionMatrix.getScaleY() &&
            initialPositionMatrix.getSkewX()  == positionMatrix.getSkewX()  &&
            initialPositionMatrix.getSkewY()  == positionMatrix.getSkewY()  &&
            SkScalarIsInt(translation.x()) && SkScalarIsInt(translation.y()),
            translation};
}


static SkColor compute_canonical_color(const SkPaint& paint, bool lcd) {
    SkColor canonicalColor = SkPaintPriv::ComputeLuminanceColor(paint);
    if (lcd) {
        // This is the correct computation for canonicalColor, but there are tons of cases where LCD
        // can be modified. For now we just regenerate if any run in a textblob has LCD.
        // TODO figure out where all of these modifications are and see if we can incorporate that
        //      logic at a higher level *OR* use sRGB
        //canonicalColor = SkMaskGamma::CanonicalColor(canonicalColor);

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note above.  We pick a placeholder value for LCD text to ensure we always match
        // the same key
        return SK_ColorTRANSPARENT;
    } else {
        // A8, though can have mixed BMP text but it shouldn't matter because BMP text won't have
        // gamma corrected masks anyways, nor color
        U8CPU lum = SkComputeLuminance(SkColorGetR(canonicalColor),
                                       SkColorGetG(canonicalColor),
                                       SkColorGetB(canonicalColor));
        // reduce to our finite number of bits
        canonicalColor = SkMaskGamma::CanonicalColor(SkColorSetRGB(lum, lum, lum));
    }
    return canonicalColor;
}

// -- SlugImpl -------------------------------------------------------------------------------------
class SlugImpl final : public Slug {
public:
    SlugImpl(SubRunAllocator&& alloc,
             SubRunContainerOwner subRuns,
             SkRect sourceBounds,
             const SkPaint& paint,
             SkPoint origin);
    ~SlugImpl() override = default;

    static sk_sp<SlugImpl> Make(const SkMatrixProvider& viewMatrix,
                                const sktext::GlyphRunList& glyphRunList,
                                const SkPaint& initialPaint,
                                const SkPaint& drawingPaint,
                                SkStrikeDeviceInfo strikeDeviceInfo,
                                sktext::StrikeForGPUCacheInterface* strikeCache);
    static sk_sp<Slug> MakeFromBuffer(SkReadBuffer& buffer,
                                      const SkStrikeClient* client);
    void doFlatten(SkWriteBuffer& buffer) const override;

#if SK_SUPPORT_GPU
    void surfaceDraw(SkCanvas*,
                     const GrClip* clip,
                     const SkMatrixProvider& viewMatrix,
                     const SkPaint& paint,
                     skgpu::v1::SurfaceDrawContext* sdc) const;
#endif

    SkRect sourceBounds() const override { return fSourceBounds; }
    SkRect sourceBoundsWithOrigin() const override { return fSourceBounds.makeOffset(fOrigin); }
    const SkPaint& initialPaint() const override { return fInitialPaint; }

    const SkMatrix& initialPositionMatrix() const { return fSubRuns->initialPosition(); }
    SkPoint origin() const { return fOrigin; }

    // Change memory management to handle the data after Slug, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p) { ::operator delete(p); }
    void* operator new(size_t) { SK_ABORT("All slugs are created by placement new."); }
    void* operator new(size_t, void* p) { return p; }

private:
    // The allocator must come first because it needs to be destroyed last. Other fields of this
    // structure may have pointers into it.
    SubRunAllocator fAlloc;
    SubRunContainerOwner fSubRuns;
    const SkRect fSourceBounds;
    const SkPaint fInitialPaint;
    const SkMatrix fInitialPositionMatrix;
    const SkPoint fOrigin;
};

SlugImpl::SlugImpl(SubRunAllocator&& alloc,
                   SubRunContainerOwner subRuns,
                   SkRect sourceBounds,
                   const SkPaint& paint,
                   SkPoint origin)
        : fAlloc {std::move(alloc)}
        , fSubRuns(std::move(subRuns))
        , fSourceBounds{sourceBounds}
        , fInitialPaint{paint}
        , fOrigin{origin} {}

#if SK_SUPPORT_GPU
void SlugImpl::surfaceDraw(SkCanvas* canvas, const GrClip* clip, const SkMatrixProvider& viewMatrix,
                           const SkPaint& drawingPaint, skgpu::v1::SurfaceDrawContext* sdc) const {
    fSubRuns->draw(canvas, clip, viewMatrix, fOrigin, drawingPaint, this, sdc);
}
#endif

void SlugImpl::doFlatten(SkWriteBuffer& buffer) const {
    buffer.writeRect(fSourceBounds);
    SkPaintPriv::Flatten(fInitialPaint, buffer);
    buffer.writePoint(fOrigin);
    fSubRuns->flattenAllocSizeHint(buffer);
    fSubRuns->flattenRuns(buffer);
}

sk_sp<Slug> SlugImpl::MakeFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client) {
    SkRect sourceBounds = buffer.readRect();
    SkASSERT(!sourceBounds.isEmpty());
    if (!buffer.validate(!sourceBounds.isEmpty())) { return nullptr; }
    SkPaint paint = buffer.readPaint();
    SkPoint origin = buffer.readPoint();
    int allocSizeHint = SubRunContainer::AllocSizeHintFromBuffer(buffer);

    auto [initializer, _, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<SlugImpl>(allocSizeHint);

    SubRunContainerOwner container = SubRunContainer::MakeFromBufferInAlloc(buffer, client, &alloc);

    // Something went wrong while reading.
    SkASSERT(buffer.isValid());
    if (!buffer.isValid()) { return nullptr;}

    return sk_sp<SlugImpl>(initializer.initialize(
            std::move(alloc), std::move(container), sourceBounds, paint, origin));
}

sk_sp<SlugImpl> SlugImpl::Make(const SkMatrixProvider& viewMatrix,
                               const sktext::GlyphRunList& glyphRunList,
                               const SkPaint& initialPaint,
                               const SkPaint& drawingPaint,
                               SkStrikeDeviceInfo strikeDeviceInfo,
                               sktext::StrikeForGPUCacheInterface* strikeCache) {
    size_t subRunSizeHint = SubRunContainer::EstimateAllocSize(glyphRunList);
    auto [initializer, _, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<SlugImpl>(subRunSizeHint);

    const SkMatrix positionMatrix =
            position_matrix(viewMatrix.localToDevice(), glyphRunList.origin());

    auto subRuns = SubRunContainer::MakeInAlloc(glyphRunList,
                                                positionMatrix,
                                                drawingPaint,
                                                strikeDeviceInfo,
                                                strikeCache,
                                                &alloc,
                                                SubRunContainer::kAddSubRuns,
                                                "Make Slug");

    sk_sp<SlugImpl> slug = sk_sp<SlugImpl>(initializer.initialize(
            std::move(alloc),
            std::move(subRuns),
            glyphRunList.sourceBounds(),
            initialPaint,
            glyphRunList.origin()));

    // There is nothing to draw here. This is particularly a problem with RSX form blobs where a
    // single space becomes a run with no glyphs.
    if (slug->fSubRuns->isEmpty()) { return nullptr; }

    return slug;
}
}  // namespace

namespace sktext::gpu {
// -- TextBlob::Key ------------------------------------------------------------------------------
auto TextBlob::Key::Make(const GlyphRunList& glyphRunList,
                         const SkPaint& paint,
                         const SkMatrix& drawMatrix,
                         const SkStrikeDeviceInfo& strikeDevice) -> std::tuple<bool, Key> {
    SkASSERT(strikeDevice.fSDFTControl != nullptr);
    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* maskFilter = paint.getMaskFilter();
    bool canCache = glyphRunList.canCache() &&
                    !(paint.getPathEffect() ||
                        (maskFilter && !as_MFB(maskFilter)->asABlur(&blurRec)));

    TextBlob::Key key;
    if (canCache) {
        bool hasLCD = glyphRunList.anyRunsLCD();

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? strikeDevice.fSurfaceProps.pixelGeometry()
                                               : kUnknown_SkPixelGeometry;

        SkColor canonicalColor = compute_canonical_color(paint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = glyphRunList.uniqueID();
        key.fStyle = paint.getStyle();
        if (key.fStyle != SkPaint::kFill_Style) {
            key.fFrameWidth = paint.getStrokeWidth();
            key.fMiterLimit = paint.getStrokeMiter();
            key.fJoin = paint.getStrokeJoin();
        }
        key.fHasBlur = maskFilter != nullptr;
        if (key.fHasBlur) {
            key.fBlurRec = blurRec;
        }
        key.fCanonicalColor = canonicalColor;
        key.fScalerContextFlags = SkTo<uint32_t>(strikeDevice.fScalerContextFlags);

        // Do any runs use direct drawing types?.
        key.fHasSomeDirectSubRuns = false;
        SkPoint glyphRunListLocation = glyphRunList.sourceBoundsWithOrigin().center();
        for (auto& run : glyphRunList) {
            SkScalar approximateDeviceTextSize =
                    SkFontPriv::ApproximateTransformedTextSize(run.font(), drawMatrix,
                                                               glyphRunListLocation);
            key.fHasSomeDirectSubRuns |=
                    strikeDevice.fSDFTControl->isDirect(approximateDeviceTextSize, paint,
                                                        drawMatrix);
        }

        if (key.fHasSomeDirectSubRuns) {
            // Store the fractional offset of the position. We know that the matrix can't be
            // perspective at this point.
            SkPoint mappedOrigin = drawMatrix.mapOrigin();
            key.fPositionMatrix = drawMatrix;
            key.fPositionMatrix.setTranslateX(
                    mappedOrigin.x() - SkScalarFloorToScalar(mappedOrigin.x()));
            key.fPositionMatrix.setTranslateY(
                    mappedOrigin.y() - SkScalarFloorToScalar(mappedOrigin.y()));
        } else {
            // For path and SDFT, the matrix doesn't matter.
            key.fPositionMatrix = SkMatrix::I();
        }
    }

    return {canCache, key};
}

bool TextBlob::Key::operator==(const TextBlob::Key& that) const {
    if (fUniqueID != that.fUniqueID) { return false; }
    if (fCanonicalColor != that.fCanonicalColor) { return false; }
    if (fStyle != that.fStyle) { return false; }
    if (fStyle != SkPaint::kFill_Style) {
        if (fFrameWidth != that.fFrameWidth ||
            fMiterLimit != that.fMiterLimit ||
            fJoin != that.fJoin) {
            return false;
        }
    }
    if (fPixelGeometry != that.fPixelGeometry) { return false; }
    if (fHasBlur != that.fHasBlur) { return false; }
    if (fHasBlur) {
        if (fBlurRec.fStyle != that.fBlurRec.fStyle || fBlurRec.fSigma != that.fBlurRec.fSigma) {
            return false;
        }
    }

    if (fScalerContextFlags != that.fScalerContextFlags) { return false; }

    // DirectSubRuns do not support perspective when used with a TextBlob. SDFT, Transformed,
    // Path, and Drawable do support perspective.
    if (fPositionMatrix.hasPerspective() && fHasSomeDirectSubRuns) { return false; }

    if (fHasSomeDirectSubRuns != that.fHasSomeDirectSubRuns) { return false; }

    if (fHasSomeDirectSubRuns) {
        auto [compatible, _] = can_use_direct(fPositionMatrix, that.fPositionMatrix);
        return compatible;
    }

    return true;
}

// -- TextBlob -----------------------------------------------------------------------------------
void TextBlob::operator delete(void* p) { ::operator delete(p); }
void* TextBlob::operator new(size_t) { SK_ABORT("All blobs are created by placement new."); }
void* TextBlob::operator new(size_t, void* p) { return p; }

TextBlob::~TextBlob() = default;

sk_sp<TextBlob> TextBlob::Make(const GlyphRunList& glyphRunList,
                               const SkPaint& paint,
                               const SkMatrix& positionMatrix,
                               SkStrikeDeviceInfo strikeDeviceInfo,
                               StrikeForGPUCacheInterface* strikeCache) {
    size_t subRunSizeHint = SubRunContainer::EstimateAllocSize(glyphRunList);
    auto [initializer, totalMemoryAllocated, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<TextBlob>(subRunSizeHint);

    auto container = SubRunContainer::MakeInAlloc(
            glyphRunList, positionMatrix, paint,
            strikeDeviceInfo, strikeCache, &alloc, SubRunContainer::kAddSubRuns, "TextBlob");

    SkColor initialLuminance = SkPaintPriv::ComputeLuminanceColor(paint);
    sk_sp<TextBlob> blob = sk_sp<TextBlob>(initializer.initialize(std::move(alloc),
                                                                  std::move(container),
                                                                  totalMemoryAllocated,
                                                                  initialLuminance));
    return blob;
}

void TextBlob::addKey(const Key& key) {
    fKey = key;
}

bool TextBlob::hasPerspective() const {
    return fSubRuns->initialPosition().hasPerspective();
}

bool TextBlob::canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const {
    // A singular matrix will create a TextBlob with no SubRuns, but unknown glyphs can also
    // cause empty runs. If there are no subRuns, then regenerate when the matrices don't match.
    if (fSubRuns->isEmpty() && fSubRuns->initialPosition() != positionMatrix) {
        return false;
    }

    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        fInitialLuminance != SkPaintPriv::ComputeLuminanceColor(paint)) {
        return false;
    }

    return fSubRuns->canReuse(paint, positionMatrix);
}

const TextBlob::Key& TextBlob::key() const { return fKey; }

#if SK_SUPPORT_GPU
void TextBlob::draw(SkCanvas* canvas,
                    const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    SkPoint drawOrigin,
                    const SkPaint& paint,
                    skgpu::v1::SurfaceDrawContext* sdc) {
    fSubRuns->draw(canvas, clip, viewMatrix, drawOrigin, paint, this, sdc);
}
#endif

#if defined(SK_GRAPHITE_ENABLED)
void TextBlob::draw(SkCanvas* canvas,
                    SkPoint drawOrigin,
                    const SkPaint& paint,
                    skgpu::graphite::Device* device) {
    fSubRuns->draw(canvas, drawOrigin, paint, this, device);
}
#endif

#if GR_TEST_UTILS
struct SubRunContainerPeer {
    static const AtlasSubRun* getAtlasSubRun(const SubRunContainer& subRuns) {
        if (subRuns.isEmpty()) {
            return nullptr;
        }
        return subRuns.fSubRuns.front().testingOnly_atlasSubRun();
    }
};
#endif

const AtlasSubRun* TextBlob::testingOnlyFirstSubRun() const {
#if GR_TEST_UTILS
    return SubRunContainerPeer::getAtlasSubRun(*fSubRuns);
#else
    return nullptr;
#endif
}

TextBlob::TextBlob(SubRunAllocator&& alloc,
                   SubRunContainerOwner subRuns,
                   int totalMemorySize,
                   SkColor initialLuminance)
        : fAlloc{std::move(alloc)}
        , fSubRuns{std::move(subRuns)}
        , fSize(totalMemorySize)
        , fInitialLuminance{initialLuminance} { }

sk_sp<Slug> SkMakeSlugFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client) {
    return SlugImpl::MakeFromBuffer(buffer, client);
}
}  // namespace sktext::gpu

#if SK_SUPPORT_GPU
namespace skgpu::v1 {
sk_sp<Slug>
Device::convertGlyphRunListToSlug(const sktext::GlyphRunList& glyphRunList,
                                  const SkPaint& initialPaint,
                                  const SkPaint& drawingPaint) {
    return SlugImpl::Make(this->asMatrixProvider(),
                          glyphRunList,
                          initialPaint,
                          drawingPaint,
                          this->strikeDeviceInfo(),
                          SkStrikeCache::GlobalStrikeCache());
}

void Device::drawSlug(SkCanvas* canvas, const Slug* slug, const SkPaint& drawingPaint) {
    const SlugImpl* slugImpl = static_cast<const SlugImpl*>(slug);
    auto matrixProvider = this->asMatrixProvider();
#if defined(SK_DEBUG)
    if (!fContext->priv().options().fSupportBilerpFromGlyphAtlas) {
        // We can draw a slug if the atlas has padding or if the creation matrix and the
        // drawing matrix are the same. If they are the same, then the Slug will use the direct
        // drawing code and not use bi-lerp.
        SkMatrix slugMatrix = slugImpl->initialPositionMatrix();
        SkMatrix positionMatrix = matrixProvider.localToDevice();
        positionMatrix.preTranslate(slugImpl->origin().x(), slugImpl->origin().y());
        SkASSERT(slugMatrix == positionMatrix);
    }
#endif
    slugImpl->surfaceDraw(
            canvas, this->clip(), matrixProvider, drawingPaint, fSurfaceDrawContext.get());
}

sk_sp<Slug> MakeSlug(const SkMatrixProvider& drawMatrix,
                     const sktext::GlyphRunList& glyphRunList,
                     const SkPaint& initialPaint,
                     const SkPaint& drawingPaint,
                     SkStrikeDeviceInfo strikeDeviceInfo,
                     sktext::StrikeForGPUCacheInterface* strikeCache) {
    return SlugImpl::Make(
            drawMatrix, glyphRunList, initialPaint, drawingPaint, strikeDeviceInfo, strikeCache);
}
}  // namespace skgpu::v1
#endif
