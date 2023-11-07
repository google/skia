/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyph.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPicture.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkBezierCurves.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/StrikeForGPU.h"

#include <cstring>
#include <optional>
#include <tuple>
#include <utility>

using namespace skglyph;
using namespace sktext;

// -- SkPictureBackedGlyphDrawable -----------------------------------------------------------------
sk_sp<SkPictureBackedGlyphDrawable>
SkPictureBackedGlyphDrawable::MakeFromBuffer(SkReadBuffer& buffer) {
    SkASSERT(buffer.isValid());

    sk_sp<SkData> pictureData = buffer.readByteArrayAsData();

    // Return nullptr if invalid or there an empty drawable, which is represented by nullptr.
    if (!buffer.isValid() || pictureData->size() == 0) {
        return nullptr;
    }

    // Propagate the outer buffer's allow-SkSL setting to the picture decoder, using the flag on
    // the deserial procs.
    SkDeserialProcs procs;
    procs.fAllowSkSL = buffer.allowSkSL();
    sk_sp<SkPicture> picture = SkPicture::MakeFromData(pictureData.get(), &procs);
    if (!buffer.validate(picture != nullptr)) {
        return nullptr;
    }

    return sk_make_sp<SkPictureBackedGlyphDrawable>(std::move(picture));
}

void SkPictureBackedGlyphDrawable::FlattenDrawable(SkWriteBuffer& buffer, SkDrawable* drawable) {
    if (drawable == nullptr) {
        buffer.writeByteArray(nullptr, 0);
        return;
    }

    sk_sp<SkPicture> picture = drawable->makePictureSnapshot();
    // These drawables should not have SkImages, SkTypefaces or SkPictures inside of them, so
    // the default SkSerialProcs are sufficient.
    sk_sp<SkData> data = picture->serialize();

    // If the picture is too big, or there is no picture, then drop by sending an empty byte array.
    if (!SkTFitsIn<uint32_t>(data->size()) || data->size() == 0) {
        buffer.writeByteArray(nullptr, 0);
        return;
    }

    buffer.writeByteArray(data->data(), data->size());
}

SkPictureBackedGlyphDrawable::SkPictureBackedGlyphDrawable(sk_sp<SkPicture> picture)
        : fPicture(std::move(picture)) {}

SkRect SkPictureBackedGlyphDrawable::onGetBounds() {
    return fPicture->cullRect();
}

size_t SkPictureBackedGlyphDrawable::onApproximateBytesUsed() {
    return sizeof(SkPictureBackedGlyphDrawable) + fPicture->approximateBytesUsed();
}

void SkPictureBackedGlyphDrawable::onDraw(SkCanvas* canvas) {
    canvas->drawPicture(fPicture);
}

//-- SkGlyph ---------------------------------------------------------------------------------------
std::optional<SkGlyph> SkGlyph::MakeFromBuffer(SkReadBuffer& buffer) {
    SkASSERT(buffer.isValid());
    const SkPackedGlyphID packedID{buffer.readUInt()};
    const SkVector advance = buffer.readPoint();
    const uint32_t dimensions = buffer.readUInt();
    const uint32_t leftTop = buffer.readUInt();
    const SkMask::Format format = SkTo<SkMask::Format>(buffer.readUInt());

    if (!buffer.validate(SkMask::IsValidFormat(format))) {
        return std::nullopt;
    }

    SkGlyph glyph{packedID};
    glyph.fAdvanceX = advance.x();
    glyph.fAdvanceY = advance.y();
    glyph.fWidth = dimensions >> 16;
    glyph.fHeight = dimensions & 0xffffu;
    glyph.fLeft = leftTop >> 16;
    glyph.fTop = leftTop & 0xffffu;
    glyph.fMaskFormat = format;
    SkDEBUGCODE(glyph.fAdvancesBoundsFormatAndInitialPathDone = true;)
    return glyph;
}

SkGlyph::SkGlyph(const SkGlyph&) = default;
SkGlyph& SkGlyph::operator=(const SkGlyph&) = default;
SkGlyph::SkGlyph(SkGlyph&&) = default;
SkGlyph& SkGlyph::operator=(SkGlyph&&) = default;
SkGlyph::~SkGlyph() = default;

SkMask SkGlyph::mask() const {
    SkIRect bounds = SkIRect::MakeXYWH(fLeft, fTop, fWidth, fHeight);
    return SkMask(static_cast<const uint8_t*>(fImage), bounds, this->rowBytes(), fMaskFormat);
}

SkMask SkGlyph::mask(SkPoint position) const {
    SkASSERT(SkScalarIsInt(position.x()) && SkScalarIsInt(position.y()));
    SkIRect bounds = SkIRect::MakeXYWH(fLeft, fTop, fWidth, fHeight);
    bounds.offset(SkScalarFloorToInt(position.x()), SkScalarFloorToInt(position.y()));
    return SkMask(static_cast<const uint8_t*>(fImage), bounds, this->rowBytes(), fMaskFormat);
}

void SkGlyph::zeroMetrics() {
    fAdvanceX = 0;
    fAdvanceY = 0;
    fWidth    = 0;
    fHeight   = 0;
    fTop      = 0;
    fLeft     = 0;
}

static size_t bits_to_bytes(size_t bits) {
    return (bits + 7) >> 3;
}

static size_t format_alignment(SkMask::Format format) {
    switch (format) {
        case SkMask::kBW_Format:
        case SkMask::kA8_Format:
        case SkMask::k3D_Format:
        case SkMask::kSDF_Format:
            return alignof(uint8_t);
        case SkMask::kARGB32_Format:
            return alignof(uint32_t);
        case SkMask::kLCD16_Format:
            return alignof(uint16_t);
        default:
            SK_ABORT("Unknown mask format.");
            break;
    }
    return 0;
}

static size_t format_rowbytes(int width, SkMask::Format format) {
    return format == SkMask::kBW_Format ? bits_to_bytes(width)
                                        : width * format_alignment(format);
}

size_t SkGlyph::formatAlignment() const {
    return format_alignment(this->maskFormat());
}

size_t SkGlyph::allocImage(SkArenaAlloc* alloc) {
    SkASSERT(!this->isEmpty());
    auto size = this->imageSize();
    fImage = alloc->makeBytesAlignedTo(size, this->formatAlignment());

    return size;
}

bool SkGlyph::setImage(SkArenaAlloc* alloc, SkScalerContext* scalerContext) {
    if (!this->setImageHasBeenCalled()) {
        // It used to be that getImage() could change the fMaskFormat. Extra checking to make
        // sure there are no regressions.
        SkDEBUGCODE(SkMask::Format oldFormat = this->maskFormat());
        this->allocImage(alloc);
        scalerContext->getImage(*this);
        SkASSERT(oldFormat == this->maskFormat());
        return true;
    }
    return false;
}

bool SkGlyph::setImage(SkArenaAlloc* alloc, const void* image) {
    if (!this->setImageHasBeenCalled()) {
        this->allocImage(alloc);
        memcpy(fImage, image, this->imageSize());
        return true;
    }
    return false;
}

size_t SkGlyph::setMetricsAndImage(SkArenaAlloc* alloc, const SkGlyph& from) {
    // Since the code no longer tries to find replacement glyphs, the image should always be
    // nullptr.
    SkASSERT(fImage == nullptr || from.fImage == nullptr);

    // TODO(herb): remove "if" when we are sure there are no colliding glyphs.
    if (fImage == nullptr) {
        fAdvanceX = from.fAdvanceX;
        fAdvanceY = from.fAdvanceY;
        fWidth = from.fWidth;
        fHeight = from.fHeight;
        fTop = from.fTop;
        fLeft = from.fLeft;
        fScalerContextBits = from.fScalerContextBits;
        fMaskFormat = from.fMaskFormat;

        // From glyph may not have an image because the glyph is too large.
        if (from.fImage != nullptr && this->setImage(alloc, from.image())) {
            return this->imageSize();
        }

        SkDEBUGCODE(fAdvancesBoundsFormatAndInitialPathDone = from.fAdvancesBoundsFormatAndInitialPathDone;)
    }
    return 0;
}

size_t SkGlyph::rowBytes() const {
    return format_rowbytes(fWidth, fMaskFormat);
}

size_t SkGlyph::rowBytesUsingFormat(SkMask::Format format) const {
    return format_rowbytes(fWidth, format);
}

size_t SkGlyph::imageSize() const {
    if (this->isEmpty() || this->imageTooLarge()) { return 0; }

    size_t size = this->rowBytes() * fHeight;

    if (fMaskFormat == SkMask::k3D_Format) {
        size *= 3;
    }

    return size;
}

void SkGlyph::installPath(SkArenaAlloc* alloc, const SkPath* path, bool hairline) {
    SkASSERT(fPathData == nullptr);
    SkASSERT(!this->setPathHasBeenCalled());
    fPathData = alloc->make<SkGlyph::PathData>();
    if (path != nullptr) {
        fPathData->fPath = *path;
        fPathData->fPath.updateBoundsCache();
        fPathData->fPath.getGenerationID();
        fPathData->fHasPath = true;
        fPathData->fHairline = hairline;
    }
}

bool SkGlyph::setPath(SkArenaAlloc* alloc, SkScalerContext* scalerContext) {
    if (!this->setPathHasBeenCalled()) {
        scalerContext->getPath(*this, alloc);
        SkASSERT(this->setPathHasBeenCalled());
        return this->path() != nullptr;
    }

    return false;
}

bool SkGlyph::setPath(SkArenaAlloc* alloc, const SkPath* path, bool hairline) {
    if (!this->setPathHasBeenCalled()) {
        this->installPath(alloc, path, hairline);
        return this->path() != nullptr;
    }
    return false;
}

const SkPath* SkGlyph::path() const {
    // setPath must have been called previously.
    SkASSERT(this->setPathHasBeenCalled());
    if (fPathData->fHasPath) {
        return &fPathData->fPath;
    }
    return nullptr;
}

bool SkGlyph::pathIsHairline() const {
    // setPath must have been called previously.
    SkASSERT(this->setPathHasBeenCalled());
    return fPathData->fHairline;
}

void SkGlyph::installDrawable(SkArenaAlloc* alloc, sk_sp<SkDrawable> drawable) {
    SkASSERT(fDrawableData == nullptr);
    SkASSERT(!this->setDrawableHasBeenCalled());
    fDrawableData = alloc->make<SkGlyph::DrawableData>();
    if (drawable != nullptr) {
        fDrawableData->fDrawable = std::move(drawable);
        fDrawableData->fDrawable->getGenerationID();
        fDrawableData->fHasDrawable = true;
    }
}

bool SkGlyph::setDrawable(SkArenaAlloc* alloc, SkScalerContext* scalerContext) {
    if (!this->setDrawableHasBeenCalled()) {
        sk_sp<SkDrawable> drawable = scalerContext->getDrawable(*this);
        this->installDrawable(alloc, std::move(drawable));
        return this->drawable() != nullptr;
    }
    return false;
}

bool SkGlyph::setDrawable(SkArenaAlloc* alloc, sk_sp<SkDrawable> drawable) {
    if (!this->setDrawableHasBeenCalled()) {
        this->installDrawable(alloc, std::move(drawable));
        return this->drawable() != nullptr;
    }
    return false;
}

SkDrawable* SkGlyph::drawable() const {
    // setDrawable must have been called previously.
    SkASSERT(this->setDrawableHasBeenCalled());
    if (fDrawableData->fHasDrawable) {
        return fDrawableData->fDrawable.get();
    }
    return nullptr;
}

void SkGlyph::flattenMetrics(SkWriteBuffer& buffer) const {
    buffer.writeUInt(fID.value());
    buffer.writePoint({fAdvanceX, fAdvanceY});
    buffer.writeUInt(fWidth << 16 | fHeight);
    // Note: << has undefined behavior for negative values, so convert everything to the bit
    // values of uint16_t. Using the cast keeps the signed values fLeft and fTop from sign
    // extending.
    const uint32_t left = static_cast<uint16_t>(fLeft);
    const uint32_t top = static_cast<uint16_t>(fTop);
    buffer.writeUInt(left << 16 | top);
    buffer.writeUInt(SkTo<uint32_t>(fMaskFormat));
}

void SkGlyph::flattenImage(SkWriteBuffer& buffer) const {
    SkASSERT(this->setImageHasBeenCalled());

    // If the glyph is empty or too big, then no image data is sent.
    if (!this->isEmpty() && SkGlyphDigest::FitsInAtlas(*this)) {
        buffer.writeByteArray(this->image(), this->imageSize());
    }
}

size_t SkGlyph::addImageFromBuffer(SkReadBuffer& buffer, SkArenaAlloc* alloc) {
    SkASSERT(buffer.isValid());

    // If the glyph is empty or too big, then no image data is received.
    if (this->isEmpty() || !SkGlyphDigest::FitsInAtlas(*this)) {
        return 0;
    }

    size_t memoryIncrease = 0;

    void* imageData = alloc->makeBytesAlignedTo(this->imageSize(), this->formatAlignment());
    buffer.readByteArray(imageData, this->imageSize());
    if (buffer.isValid()) {
        this->installImage(imageData);
        memoryIncrease += this->imageSize();
    }

    return memoryIncrease;
}

void SkGlyph::flattenPath(SkWriteBuffer& buffer) const {
    SkASSERT(this->setPathHasBeenCalled());

    const bool hasPath = this->path() != nullptr;
    buffer.writeBool(hasPath);
    if (hasPath) {
        buffer.writeBool(this->pathIsHairline());
        buffer.writePath(*this->path());
    }
}

size_t SkGlyph::addPathFromBuffer(SkReadBuffer& buffer, SkArenaAlloc* alloc) {
    SkASSERT(buffer.isValid());

    size_t memoryIncrease = 0;
    const bool hasPath = buffer.readBool();
    // Check if the buffer is invalid, so as to not make a logical decision on invalid data.
    if (!buffer.isValid()) {
        return 0;
    }
    if (hasPath) {
        const bool pathIsHairline = buffer.readBool();
        SkPath path;
        buffer.readPath(&path);
        if (buffer.isValid()) {
            if (this->setPath(alloc, &path, pathIsHairline)) {
                memoryIncrease += path.approximateBytesUsed();
            }
        }
    } else {
        this->setPath(alloc, nullptr, false);
    }

    return memoryIncrease;
}

void SkGlyph::flattenDrawable(SkWriteBuffer& buffer) const {
    SkASSERT(this->setDrawableHasBeenCalled());

    if (this->isEmpty() || this->drawable() == nullptr) {
        SkPictureBackedGlyphDrawable::FlattenDrawable(buffer, nullptr);
        return;
    }

    SkPictureBackedGlyphDrawable::FlattenDrawable(buffer, this->drawable());
}

size_t SkGlyph::addDrawableFromBuffer(SkReadBuffer& buffer, SkArenaAlloc* alloc) {
    SkASSERT(buffer.isValid());

    sk_sp<SkDrawable> drawable = SkPictureBackedGlyphDrawable::MakeFromBuffer(buffer);
    if (!buffer.isValid()) {
        return 0;
    }

    if (this->setDrawable(alloc, std::move(drawable))) {
        return this->drawable()->approximateBytesUsed();
    }

    return 0;
}

static std::tuple<SkScalar, SkScalar> calculate_path_gap(
        SkScalar topOffset, SkScalar bottomOffset, const SkPath& path) {

    // Left and Right of an ever expanding gap around the path.
    SkScalar left  = SK_ScalarMax,
             right = SK_ScalarMin;

    auto expandGap = [&left, &right](SkScalar v) {
        left  = std::min(left, v);
        right = std::max(right, v);
    };

    // Handle all the different verbs for the path.
    SkPoint pts[4];
    auto addLine = [&](SkScalar offset) {
        SkScalar t = sk_ieee_float_divide(offset - pts[0].fY, pts[1].fY - pts[0].fY);
        if (0 <= t && t < 1) {   // this handles divide by zero above
            expandGap(pts[0].fX + t * (pts[1].fX - pts[0].fX));
        }
    };

    auto addQuad = [&](SkScalar offset) {
        SkScalar intersectionStorage[2];
        auto intersections = SkBezierQuad::IntersectWithHorizontalLine(
                SkSpan(pts, 3), offset, intersectionStorage);
        for (SkScalar x : intersections) {
            expandGap(x);
        }
    };

    auto addCubic = [&](SkScalar offset) {
        float intersectionStorage[3];
        auto intersections = SkBezierCubic::IntersectWithHorizontalLine(
                SkSpan{pts, 4}, offset, intersectionStorage);

        for(double intersection : intersections) {
            expandGap(intersection);
        }
    };

    // Handle when a verb's points are in the gap between top and bottom.
    auto addPts = [&expandGap, &pts, topOffset, bottomOffset](int ptCount) {
        for (int i = 0; i < ptCount; ++i) {
            if (topOffset < pts[i].fY && pts[i].fY < bottomOffset) {
                expandGap(pts[i].fX);
            }
        }
    };

    SkPath::Iter iter(path, false);
    SkPath::Verb verb;
    while (SkPath::kDone_Verb != (verb = iter.next(pts))) {
        switch (verb) {
            case SkPath::kMove_Verb: {
                break;
            }
            case SkPath::kLine_Verb: {
                auto [lineTop, lineBottom] = std::minmax({pts[0].fY, pts[1].fY});

                // The y-coordinates of the points intersect the top and bottom offsets.
                if (topOffset <= lineBottom && lineTop <= bottomOffset) {
                    addLine(topOffset);
                    addLine(bottomOffset);
                    addPts(2);
                }
                break;
            }
            case SkPath::kQuad_Verb: {
                auto [quadTop, quadBottom] = std::minmax({pts[0].fY, pts[1].fY, pts[2].fY});

                // The y-coordinates of the points intersect the top and bottom offsets.
                if (topOffset <= quadBottom && quadTop <= bottomOffset) {
                    addQuad(topOffset);
                    addQuad(bottomOffset);
                    addPts(3);
                }
                break;
            }
            case SkPath::kConic_Verb: {
                SkDEBUGFAIL("There should be no conic primitives in glyph outlines.");
                break;
            }
            case SkPath::kCubic_Verb: {
                auto [cubicTop, cubicBottom] =
                        std::minmax({pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY});

                // The y-coordinates of the points intersect the top and bottom offsets.
                if (topOffset <= cubicBottom && cubicTop <= bottomOffset) {
                    addCubic(topOffset);
                    addCubic(bottomOffset);
                    addPts(4);
                }
                break;
            }
            case SkPath::kClose_Verb: {
                break;
            }
            default: {
                SkDEBUGFAIL("Unknown path verb generating glyph underline.");
                break;
            }
        }
    }

    return std::tie(left, right);
}

void SkGlyph::ensureIntercepts(const SkScalar* bounds, SkScalar scale, SkScalar xPos,
                               SkScalar* array, int* count, SkArenaAlloc* alloc) {

    auto offsetResults = [scale, xPos](
            const SkGlyph::Intercept* intercept,SkScalar* array, int* count) {
        if (array) {
            array += *count;
            for (int index = 0; index < 2; index++) {
                *array++ = intercept->fInterval[index] * scale + xPos;
            }
        }
        *count += 2;
    };

    const SkGlyph::Intercept* match =
            [this](const SkScalar bounds[2]) -> const SkGlyph::Intercept* {
                if (fPathData == nullptr) {
                    return nullptr;
                }
                const SkGlyph::Intercept* intercept = fPathData->fIntercept;
                while (intercept != nullptr) {
                    if (bounds[0] == intercept->fBounds[0] && bounds[1] == intercept->fBounds[1]) {
                        return intercept;
                    }
                    intercept = intercept->fNext;
                }
                return nullptr;
            }(bounds);

    if (match != nullptr) {
        if (match->fInterval[0] < match->fInterval[1]) {
            offsetResults(match, array, count);
        }
        return;
    }

    SkGlyph::Intercept* intercept = alloc->make<SkGlyph::Intercept>();
    intercept->fNext = fPathData->fIntercept;
    intercept->fBounds[0] = bounds[0];
    intercept->fBounds[1] = bounds[1];
    intercept->fInterval[0] = SK_ScalarMax;
    intercept->fInterval[1] = SK_ScalarMin;
    fPathData->fIntercept = intercept;
    const SkPath* path = &(fPathData->fPath);
    const SkRect& pathBounds = path->getBounds();
    if (pathBounds.fBottom < bounds[0] || bounds[1] < pathBounds.fTop) {
        return;
    }

    std::tie(intercept->fInterval[0], intercept->fInterval[1])
            = calculate_path_gap(bounds[0], bounds[1], *path);

    if (intercept->fInterval[0] >= intercept->fInterval[1]) {
        intercept->fInterval[0] = SK_ScalarMax;
        intercept->fInterval[1] = SK_ScalarMin;
        return;
    }
    offsetResults(intercept, array, count);
}

namespace {
uint32_t init_actions(const SkGlyph& glyph) {
    constexpr uint32_t kAllUnset = 0;
    constexpr uint32_t kDrop = SkTo<uint32_t>(GlyphAction::kDrop);
    constexpr uint32_t kAllDrop =
            kDrop << kDirectMask |
            kDrop << kDirectMaskCPU |
            kDrop << kMask |
            kDrop << kSDFT |
            kDrop << kPath |
            kDrop << kDrawable;
    return glyph.isEmpty() ? kAllDrop : kAllUnset;
}
}  // namespace

// -- SkGlyphDigest --------------------------------------------------------------------------------
SkGlyphDigest::SkGlyphDigest(size_t index, const SkGlyph& glyph)
        : fPackedID{SkTo<uint64_t>(glyph.getPackedID().value())}
        , fIndex{SkTo<uint64_t>(index)}
        , fIsEmpty(glyph.isEmpty())
        , fFormat(glyph.maskFormat())
        , fActions{init_actions(glyph)}
        , fLeft{SkTo<int16_t>(glyph.left())}
        , fTop{SkTo<int16_t>(glyph.top())}
        , fWidth{SkTo<uint16_t>(glyph.width())}
        , fHeight{SkTo<uint16_t>(glyph.height())} {}

void SkGlyphDigest::setActionFor(skglyph::ActionType actionType,
                                 SkGlyph* glyph,
                                 StrikeForGPU* strike) {
    // We don't have to do any more if the glyph is marked as kDrop because it was isEmpty().
    if (this->actionFor(actionType) == GlyphAction::kUnset) {
        GlyphAction action = GlyphAction::kReject;
        switch (actionType) {
            case kDirectMask: {
                if (this->fitsInAtlasDirect()) {
                    action = GlyphAction::kAccept;
                }
                break;
            }
            case kDirectMaskCPU: {
                if (strike->prepareForImage(glyph)) {
                    SkASSERT(!glyph->isEmpty());
                    action = GlyphAction::kAccept;
                }
                break;
            }
            case kMask: {
                if (this->fitsInAtlasInterpolated()) {
                    action = GlyphAction::kAccept;
                }
                break;
            }
            case kSDFT: {
                if (this->fitsInAtlasDirect() &&
                    this->maskFormat() == SkMask::Format::kSDF_Format) {
                    action = GlyphAction::kAccept;
                }
                break;
            }
            case kPath: {
                if (strike->prepareForPath(glyph)) {
                    action = GlyphAction::kAccept;
                }
                break;
            }
            case kDrawable: {
                if (strike->prepareForDrawable(glyph)) {
                    action = GlyphAction::kAccept;
                }
                break;
            }
        }
        this->setAction(actionType, action);
    }
}

bool SkGlyphDigest::FitsInAtlas(const SkGlyph& glyph) {
    return glyph.maxDimension() <= kSkSideTooBigForAtlas;
}

// -- SkGlyphPositionRoundingSpec ------------------------------------------------------------------
SkVector SkGlyphPositionRoundingSpec::HalfAxisSampleFreq(
        bool isSubpixel, SkAxisAlignment axisAlignment) {
    if (!isSubpixel) {
        return {SK_ScalarHalf, SK_ScalarHalf};
    } else {
        switch (axisAlignment) {
            case SkAxisAlignment::kX:
                return {SkPackedGlyphID::kSubpixelRound, SK_ScalarHalf};
            case SkAxisAlignment::kY:
                return {SK_ScalarHalf, SkPackedGlyphID::kSubpixelRound};
            case SkAxisAlignment::kNone:
                return {SkPackedGlyphID::kSubpixelRound, SkPackedGlyphID::kSubpixelRound};
        }
    }

    // Some compilers need this.
    return {0, 0};
}

SkIPoint SkGlyphPositionRoundingSpec::IgnorePositionMask(
        bool isSubpixel, SkAxisAlignment axisAlignment) {
    return SkIPoint::Make((!isSubpixel || axisAlignment == SkAxisAlignment::kY) ? 0 : ~0,
                          (!isSubpixel || axisAlignment == SkAxisAlignment::kX) ? 0 : ~0);
}

SkIPoint SkGlyphPositionRoundingSpec::IgnorePositionFieldMask(bool isSubpixel,
                                                              SkAxisAlignment axisAlignment) {
    SkIPoint ignoreMask = IgnorePositionMask(isSubpixel, axisAlignment);
    SkIPoint answer{ignoreMask.x() & SkPackedGlyphID::kXYFieldMask.x(),
                    ignoreMask.y() & SkPackedGlyphID::kXYFieldMask.y()};
    return answer;
}

SkGlyphPositionRoundingSpec::SkGlyphPositionRoundingSpec(
        bool isSubpixel, SkAxisAlignment axisAlignment)
    : halfAxisSampleFreq{HalfAxisSampleFreq(isSubpixel, axisAlignment)}
    , ignorePositionMask{IgnorePositionMask(isSubpixel, axisAlignment)}
    , ignorePositionFieldMask {IgnorePositionFieldMask(isSubpixel, axisAlignment)} {}
