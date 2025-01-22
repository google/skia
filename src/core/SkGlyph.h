/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyph_DEFINED
#define SkGlyph_DEFINED

#include "include/core/SkDrawable.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkVx.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkMask.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>

class SkArenaAlloc;
class SkCanvas;
class SkGlyph;
class SkReadBuffer;
class SkScalerContext;
class SkWriteBuffer;
namespace sktext {
class StrikeForGPU;
}  // namespace sktext

// -- SkPackedGlyphID ------------------------------------------------------------------------------
// A combination of SkGlyphID and sub-pixel position information.
struct SkPackedGlyphID {
    inline static constexpr uint32_t kImpossibleID = ~0u;
    enum {
        // Lengths
        kGlyphIDLen     = 16u,
        kSubPixelPosLen = 2u,

        // Bit positions
        kSubPixelX = 0u,
        kGlyphID   = kSubPixelPosLen,
        kSubPixelY = kGlyphIDLen + kSubPixelPosLen,
        kEndData   = kGlyphIDLen + 2 * kSubPixelPosLen,

        // Masks
        kGlyphIDMask     = (1u << kGlyphIDLen) - 1,
        kSubPixelPosMask = (1u << kSubPixelPosLen) - 1,
        kMaskAll         = (1u << kEndData) - 1,

        // Location of sub pixel info in a fixed pointer number.
        kFixedPointBinaryPointPos = 16u,
        kFixedPointSubPixelPosBits = kFixedPointBinaryPointPos - kSubPixelPosLen,
    };

    inline static const constexpr SkScalar kSubpixelRound =
            1.f / (1u << (SkPackedGlyphID::kSubPixelPosLen + 1));

    inline static const constexpr SkIPoint kXYFieldMask{kSubPixelPosMask << kSubPixelX,
                                                        kSubPixelPosMask << kSubPixelY};

    struct Hash {
         uint32_t operator() (SkPackedGlyphID packedID) const {
            return packedID.hash();
        }
    };

    constexpr explicit SkPackedGlyphID(SkGlyphID glyphID)
            : fID{(uint32_t)glyphID << kGlyphID} { }

    constexpr SkPackedGlyphID(SkGlyphID glyphID, SkFixed x, SkFixed y)
            : fID {PackIDXY(glyphID, x, y)} { }

    constexpr SkPackedGlyphID(SkGlyphID glyphID, uint32_t x, uint32_t y)
            : fID {PackIDSubXSubY(glyphID, x, y)} { }

    SkPackedGlyphID(SkGlyphID glyphID, SkPoint pt, SkIPoint mask)
        : fID{PackIDSkPoint(glyphID, pt, mask)} { }

    constexpr explicit SkPackedGlyphID(uint32_t v) : fID{v & kMaskAll} { }
    constexpr SkPackedGlyphID() : fID{kImpossibleID} {}

    bool operator==(const SkPackedGlyphID& that) const {
        return fID == that.fID;
    }
    bool operator!=(const SkPackedGlyphID& that) const {
        return !(*this == that);
    }
    bool operator<(SkPackedGlyphID that) const {
        return this->fID < that.fID;
    }

    SkGlyphID glyphID() const {
        return (fID >> kGlyphID) & kGlyphIDMask;
    }

    uint32_t value() const {
        return fID;
    }

    SkFixed getSubXFixed() const {
        return this->subToFixed(kSubPixelX);
    }

    SkFixed getSubYFixed() const {
        return this->subToFixed(kSubPixelY);
    }

    uint32_t hash() const {
        return SkChecksum::CheapMix(fID);
    }

    SkString dump() const {
        SkString str;
        str.appendf("glyphID: %d, x: %d, y:%d", glyphID(), getSubXFixed(), getSubYFixed());
        return str;
    }

    SkString shortDump() const {
        SkString str;
        str.appendf("0x%x|%1u|%1u", this->glyphID(),
                                    this->subPixelField(kSubPixelX),
                                    this->subPixelField(kSubPixelY));
        return str;
    }

private:
    static constexpr uint32_t PackIDSubXSubY(SkGlyphID glyphID, uint32_t x, uint32_t y) {
        SkASSERT(x < (1u << kSubPixelPosLen));
        SkASSERT(y < (1u << kSubPixelPosLen));

        return (x << kSubPixelX) | (y << kSubPixelY) | (glyphID << kGlyphID);
    }

    // Assumptions: pt is properly rounded. mask is set for the x or y fields.
    //
    // A sub-pixel field is a number on the interval [2^kSubPixel, 2^(kSubPixel + kSubPixelPosLen)).
    // Where kSubPixel is either kSubPixelX or kSubPixelY. Given a number x on [0, 1) we can
    // generate a sub-pixel field using:
    //    sub-pixel-field = x * 2^(kSubPixel + kSubPixelPosLen)
    //
    // We can generate the integer sub-pixel field by &-ing the integer part of sub-filed with the
    // sub-pixel field mask.
    //    int-sub-pixel-field = int(sub-pixel-field) & (kSubPixelPosMask << kSubPixel)
    //
    // The last trick is to extend the range from [0, 1) to [0, 2). The extend range is
    // necessary because the modulo 1 calculation (pt - floor(pt)) generates numbers on [-1, 1).
    // This does not round (floor) properly when converting to integer. Adding one to the range
    // causes truncation and floor to be the same. Coincidentally, masking to produce the field also
    // removes the +1.
    static uint32_t PackIDSkPoint(SkGlyphID glyphID, SkPoint pt, SkIPoint mask) {
    #if 0
        // TODO: why does this code not work on GCC 8.3 x86 Debug builds?
        using namespace skvx;
        using XY = Vec<2, float>;
        using SubXY = Vec<2, int>;

        const XY magic = {1.f * (1u << (kSubPixelPosLen + kSubPixelX)),
                          1.f * (1u << (kSubPixelPosLen + kSubPixelY))};
        XY pos{pt.x(), pt.y()};
        XY subPos = (pos - floor(pos)) + 1.0f;
        SubXY sub = cast<int>(subPos * magic) & SubXY{mask.x(), mask.y()};
    #else
        const float magicX = 1.f * (1u << (kSubPixelPosLen + kSubPixelX)),
                    magicY = 1.f * (1u << (kSubPixelPosLen + kSubPixelY));

        float x = pt.x(),
              y = pt.y();
        x = (x - floorf(x)) + 1.0f;
        y = (y - floorf(y)) + 1.0f;
        int sub[] = {
            (int)(x * magicX) & mask.x(),
            (int)(y * magicY) & mask.y(),
        };
    #endif

        SkASSERT(sub[0] / (1u << kSubPixelX) < (1u << kSubPixelPosLen));
        SkASSERT(sub[1] / (1u << kSubPixelY) < (1u << kSubPixelPosLen));
        return (glyphID << kGlyphID) | sub[0] | sub[1];
    }

    static constexpr uint32_t PackIDXY(SkGlyphID glyphID, SkFixed x, SkFixed y) {
        return PackIDSubXSubY(glyphID, FixedToSub(x), FixedToSub(y));
    }

    static constexpr uint32_t FixedToSub(SkFixed n) {
        return ((uint32_t)n >> kFixedPointSubPixelPosBits) & kSubPixelPosMask;
    }

    constexpr uint32_t subPixelField(uint32_t subPixelPosBit) const {
        return (fID >> subPixelPosBit) & kSubPixelPosMask;
    }

    constexpr SkFixed subToFixed(uint32_t subPixelPosBit) const {
        uint32_t subPixelPosition = this->subPixelField(subPixelPosBit);
        return subPixelPosition << kFixedPointSubPixelPosBits;
    }

    uint32_t fID;
};

// -- SkAxisAlignment ------------------------------------------------------------------------------
// SkAxisAlignment specifies the x component of a glyph's position is rounded when kX, and the y
// component is rounded when kY. If kNone then neither are rounded.
enum class SkAxisAlignment : uint32_t {
    kNone,
    kX,
    kY,
};

// round and ignorePositionMask are used to calculate the subpixel position of a glyph.
// The per component (x or y) calculation is:
//
//   subpixelOffset = (floor((viewportPosition + rounding) & mask) >> 14) & 3
//
// where mask is either 0 or ~0, and rounding is either
// 1/2 for non-subpixel or 1/8 for subpixel.
struct SkGlyphPositionRoundingSpec {
    SkGlyphPositionRoundingSpec(bool isSubpixel, SkAxisAlignment axisAlignment);
    const SkVector halfAxisSampleFreq;
    const SkIPoint ignorePositionMask;
    const SkIPoint ignorePositionFieldMask;

private:
    static SkVector HalfAxisSampleFreq(bool isSubpixel, SkAxisAlignment axisAlignment);
    static SkIPoint IgnorePositionMask(bool isSubpixel, SkAxisAlignment axisAlignment);
    static SkIPoint IgnorePositionFieldMask(bool isSubpixel, SkAxisAlignment axisAlignment);
};

class SkGlyphRect;
namespace skglyph {
SkGlyphRect rect_union(SkGlyphRect, SkGlyphRect);
SkGlyphRect rect_intersection(SkGlyphRect, SkGlyphRect);
}  // namespace skglyph

// SkGlyphRect encodes rectangles with coordinates using SkScalar. It is specialized for
// rectangle union and intersection operations.
class SkGlyphRect {
public:
    SkGlyphRect() = default;
    SkGlyphRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
            : fRect{-left, -top, right, bottom} { }
    bool empty() const {
        return -fRect[0] >= fRect[2] || -fRect[1] >= fRect[3];
    }
    SkRect rect() const {
        return SkRect::MakeLTRB(-fRect[0], -fRect[1], fRect[2], fRect[3]);
    }
    SkGlyphRect offset(SkScalar x, SkScalar y) const {
        return SkGlyphRect{fRect + Storage{-x, -y, x, y}};
    }
    SkGlyphRect offset(SkPoint pt) const {
        return this->offset(pt.x(), pt.y());
    }
    SkGlyphRect scaleAndOffset(SkScalar scale, SkPoint offset) const {
        auto [x, y] = offset;
        return fRect * scale + Storage{-x, -y, x, y};
    }
    SkGlyphRect inset(SkScalar dx, SkScalar dy) const {
        return fRect - Storage{dx, dy, dx, dy};
    }
    SkPoint leftTop() const { return -this->negLeftTop(); }
    SkPoint rightBottom() const { return {fRect[2], fRect[3]}; }
    SkPoint widthHeight() const { return this->rightBottom() + negLeftTop(); }
    friend SkGlyphRect skglyph::rect_union(SkGlyphRect, SkGlyphRect);
    friend SkGlyphRect skglyph::rect_intersection(SkGlyphRect, SkGlyphRect);

private:
    SkPoint negLeftTop() const { return {fRect[0], fRect[1]}; }
    using Storage = skvx::Vec<4, SkScalar>;
    SkGlyphRect(Storage rect) : fRect{rect} { }
    Storage fRect;
};

namespace skglyph {
inline SkGlyphRect empty_rect() {
    constexpr SkScalar max = std::numeric_limits<SkScalar>::max();
    return {max, max, -max, -max};
}
inline SkGlyphRect full_rect() {
    constexpr SkScalar max = std::numeric_limits<SkScalar>::max();
    return {-max, -max, max, max};
}
inline SkGlyphRect rect_union(SkGlyphRect a, SkGlyphRect b) {
    return skvx::max(a.fRect, b.fRect);
}
inline SkGlyphRect rect_intersection(SkGlyphRect a, SkGlyphRect b) {
    return skvx::min(a.fRect, b.fRect);
}

enum class GlyphAction {
    kUnset,
    kAccept,
    kReject,
    kDrop,
    kSize,
};

enum ActionType {
    kDirectMask = 0,
    kDirectMaskCPU = 2,
    kMask = 4,
    kSDFT = 6,
    kPath = 8,
    kDrawable = 10,
};

enum ActionTypeSize {
    kTotalBits = 12
};
}  // namespace skglyph

// SkGlyphDigest contains a digest of information for making GPU drawing decisions. It can be
// referenced instead of the glyph itself in many situations. In the remote glyphs cache the
// SkGlyphDigest is the only information that needs to be stored in the cache.
class SkGlyphDigest {
public:
    // An atlas consists of plots, and plots hold glyphs. The minimum a plot can be is 256x256.
    // This means that the maximum size a glyph can be is 256x256.
    static constexpr uint16_t kSkSideTooBigForAtlas = 256;

    // Default ctor is only needed for the hash table.
    SkGlyphDigest() = default;
    SkGlyphDigest(size_t index, const SkGlyph& glyph);
    int index()          const { return fIndex; }
    bool isEmpty()       const { return fIsEmpty; }
    bool isColor()       const { return fFormat == SkMask::kARGB32_Format; }
    SkMask::Format maskFormat() const { return static_cast<SkMask::Format>(fFormat); }

    skglyph::GlyphAction actionFor(skglyph::ActionType actionType) const {
        return static_cast<skglyph::GlyphAction>((fActions >> actionType) & 0b11);
    }

    void setActionFor(skglyph::ActionType, SkGlyph*, sktext::StrikeForGPU*);

    uint16_t maxDimension() const {
        return std::max(fWidth, fHeight);
    }

    bool fitsInAtlasDirect() const {
        return this->maxDimension() <= kSkSideTooBigForAtlas;
    }

    bool fitsInAtlasInterpolated() const {
        // Include the padding needed for interpolating the glyph when drawing.
        return this->maxDimension() <= kSkSideTooBigForAtlas - 2;
    }

    SkGlyphRect bounds() const {
        return SkGlyphRect(fLeft, fTop, (SkScalar)fLeft + fWidth, (SkScalar)fTop + fHeight);
    }

    static bool FitsInAtlas(const SkGlyph& glyph);

    // GetKey and Hash implement the required methods for THashTable.
    static SkPackedGlyphID GetKey(SkGlyphDigest digest) {
        return SkPackedGlyphID{SkTo<uint32_t>(digest.fPackedID)};
    }
    static uint32_t Hash(SkPackedGlyphID packedID) {
        return packedID.hash();
    }
    static bool ShouldGrow(int count, int capacity) {
        // Having the 50% load factor results in performance improvements and significantly reduces
        // the average number of probes on the Speedometer3 Editor-TipTap benchmark.
        return 2 * count >= capacity;
    }
    static bool ShouldShrink(int count, int capacity) {
        // Use 1/6 as the minimal load.
        return 6 * count <= capacity;
    }

private:
    void setAction(skglyph::ActionType actionType, skglyph::GlyphAction action) {
        using namespace skglyph;
        SkASSERT(action != GlyphAction::kUnset);
        SkASSERT(this->actionFor(actionType) == GlyphAction::kUnset);
        const uint64_t mask = 0b11 << actionType;
        fActions &= ~mask;
        fActions |= SkTo<uint64_t>(action) << actionType;
    }

    static_assert(SkPackedGlyphID::kEndData == 20);
    static_assert(SkMask::kCountMaskFormats <= 8);
    static_assert(SkTo<int>(skglyph::GlyphAction::kSize) <= 4);
    struct {
        uint64_t fPackedID : SkPackedGlyphID::kEndData;
        uint64_t fIndex    : SkPackedGlyphID::kEndData;
        uint64_t fIsEmpty  : 1;
        uint64_t fFormat   : 3;
        uint64_t fActions  : skglyph::ActionTypeSize::kTotalBits;
    };
    int16_t fLeft, fTop;
    uint16_t fWidth, fHeight;
};

class SkPictureBackedGlyphDrawable final : public SkDrawable {
public:
    static sk_sp<SkPictureBackedGlyphDrawable>MakeFromBuffer(SkReadBuffer& buffer);
    static void FlattenDrawable(SkWriteBuffer& buffer, SkDrawable* drawable);
    SkPictureBackedGlyphDrawable(sk_sp<SkPicture> self);

private:
    sk_sp<SkPicture> fPicture;
    SkRect onGetBounds() override;
    size_t onApproximateBytesUsed() override;
    void onDraw(SkCanvas* canvas) override;
};

class SkGlyph {
public:
    static std::optional<SkGlyph> MakeFromBuffer(SkReadBuffer&);
    // SkGlyph() is used for testing.
    constexpr SkGlyph() : SkGlyph{SkPackedGlyphID()} { }
    SkGlyph(const SkGlyph&) = default;
    SkGlyph& operator=(const SkGlyph&) = default;
    SkGlyph(SkGlyph&&) = default;
    SkGlyph& operator=(SkGlyph&&) = default;
    ~SkGlyph() = default;
    constexpr explicit SkGlyph(SkPackedGlyphID id) : fID{id} { }

    SkVector advanceVector() const { return SkVector{fAdvanceX, fAdvanceY}; }
    SkScalar advanceX() const { return fAdvanceX; }
    SkScalar advanceY() const { return fAdvanceY; }

    SkGlyphID getGlyphID() const { return fID.glyphID(); }
    SkPackedGlyphID getPackedID() const { return fID; }
    SkFixed getSubXFixed() const { return fID.getSubXFixed(); }
    SkFixed getSubYFixed() const { return fID.getSubYFixed(); }

    size_t rowBytes() const;
    size_t rowBytesUsingFormat(SkMask::Format format) const;

    // Call this to set all the metrics fields to 0 (e.g. if the scaler
    // encounters an error measuring a glyph). Note: this does not alter the
    // fImage, fPath, fID, fMaskFormat fields.
    void zeroMetrics();

    SkMask mask() const;

    SkMask mask(SkPoint position) const;

    // Image
    // If we haven't already tried to associate an image with this glyph
    // (i.e. setImageHasBeenCalled() returns false), then use the
    // SkScalerContext or const void* argument to set the image.
    bool setImage(SkArenaAlloc* alloc, SkScalerContext* scalerContext);
    bool setImage(SkArenaAlloc* alloc, const void* image);

    // Merge the 'from' glyph into this glyph using alloc to allocate image data. Return the number
    // of bytes allocated. Copy the width, height, top, left, format, and image into this glyph
    // making a copy of the image using the alloc.
    size_t setMetricsAndImage(SkArenaAlloc* alloc, const SkGlyph& from);

    // Returns true if the image has been set.
    bool setImageHasBeenCalled() const {
        // Check for empty bounds first to guard against fImage somehow being set.
        return this->isEmpty() || fImage != nullptr || this->imageTooLarge();
    }

    // Return a pointer to the path if the image exists, otherwise return nullptr.
    const void* image() const { SkASSERT(this->setImageHasBeenCalled()); return fImage; }

    // Return the size of the image.
    size_t imageSize() const;

    // Path
    // If we haven't already tried to associate a path to this glyph
    // (i.e. setPathHasBeenCalled() returns false), then use the
    // SkScalerContext or SkPath argument to try to do so.  N.B. this
    // may still result in no path being associated with this glyph,
    // e.g. if you pass a null SkPath or the typeface is bitmap-only.
    //
    // This setPath() call is sticky... once you call it, the glyph
    // stays in its state permanently, ignoring any future calls.
    //
    // Returns true if this is the first time you called setPath()
    // and there actually is a path; call path() to get it.
    bool setPath(SkArenaAlloc* alloc, SkScalerContext* scalerContext);
    bool setPath(SkArenaAlloc* alloc, const SkPath* path, bool hairline, bool modified);

    // Returns true if that path has been set.
    bool setPathHasBeenCalled() const { return fPathData != nullptr; }

    // Return a pointer to the path if it exists, otherwise return nullptr. Only works if the
    // path was previously set.
    const SkPath* path() const;
    bool pathIsHairline() const;
    bool pathIsModified() const;

    bool setDrawable(SkArenaAlloc* alloc, SkScalerContext* scalerContext);
    bool setDrawable(SkArenaAlloc* alloc, sk_sp<SkDrawable> drawable);
    bool setDrawableHasBeenCalled() const { return fDrawableData != nullptr; }
    SkDrawable* drawable() const;

    // Format
    bool isColor() const { return fMaskFormat == SkMask::kARGB32_Format; }
    SkMask::Format maskFormat() const { return fMaskFormat; }
    size_t formatAlignment() const;

    // Bounds
    int maxDimension() const { return std::max(fWidth, fHeight); }
    SkIRect iRect() const { return SkIRect::MakeXYWH(fLeft, fTop, fWidth, fHeight); }
    SkRect rect()   const { return SkRect::MakeXYWH(fLeft, fTop, fWidth, fHeight);  }
    SkGlyphRect glyphRect() const {
        return SkGlyphRect(fLeft, fTop, fLeft + fWidth, fTop + fHeight);
    }
    int left()   const { return fLeft;   }
    int top()    const { return fTop;    }
    int width()  const { return fWidth;  }
    int height() const { return fHeight; }
    bool isEmpty() const {
        return fWidth == 0 || fHeight == 0;
    }
    bool imageTooLarge() const { return fWidth >= kMaxGlyphWidth; }

    uint16_t extraBits() const { return fScalerContextBits; }

    // Make sure that the intercept information is on the glyph and return it, or return it if it
    // already exists.
    // * bounds - [0] - top of underline; [1] - bottom of underline.
    // * scale, xPos - information about how wide the gap is.
    // * array - accumulated gaps for many characters if not null.
    // * count - the number of gaps.
    void ensureIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                          SkScalar* array, int* count, SkArenaAlloc* alloc);

    // Deprecated. Do not use. The last use is in SkChromeRemoteCache, and will be deleted soon.
    void setImage(void* image) { fImage = image; }

    // Serialize/deserialize functions.
    // Flatten the metrics portions, but no drawing data.
    void flattenMetrics(SkWriteBuffer&) const;

    // Flatten just the the mask data.
    void flattenImage(SkWriteBuffer&) const;

    // Read the image data, store it in the alloc, and add it to the glyph.
    size_t addImageFromBuffer(SkReadBuffer&, SkArenaAlloc*);

    // Flatten just the path data.
    void flattenPath(SkWriteBuffer&) const;

    // Read the path data, create the glyph's path data in the alloc, and add it to the glyph.
    size_t addPathFromBuffer(SkReadBuffer&, SkArenaAlloc*);

    // Flatten just the drawable data.
    void flattenDrawable(SkWriteBuffer&) const;

    // Read the drawable data, create the glyph's drawable data in the alloc, and add it to the
    // glyph.
    size_t addDrawableFromBuffer(SkReadBuffer&, SkArenaAlloc*);

private:
    // There are two sides to an SkGlyph, the scaler side (things that create glyph data) have
    // access to all the fields. Scalers are assumed to maintain all the SkGlyph invariants. The
    // consumer side has a tighter interface.
    friend class SkScalerContext;
    friend class SkGlyphTestPeer;

    inline static constexpr uint16_t kMaxGlyphWidth = 1u << 13u;

    // Support horizontal and vertical skipping strike-through / underlines.
    // The caller walks the linked list looking for a match. For a horizontal underline,
    // the fBounds contains the top and bottom of the underline. The fInterval pair contains the
    // beginning and end of the intersection of the bounds and the glyph's path.
    // If interval[0] >= interval[1], no intersection was found.
    struct Intercept {
        Intercept* fNext;
        SkScalar   fBounds[2];    // for horz underlines, the boundaries in Y
        SkScalar   fInterval[2];  // the outside intersections of the axis and the glyph
    };

    struct PathData {
        Intercept* fIntercept{nullptr};
        SkPath     fPath;
        bool       fHasPath{false};
        // A normal user-path will have patheffects applied to it and eventually become a dev-path.
        // A dev-path is always a fill-path, except when it is hairline.
        // The fPath is a dev-path, so sidecar the paths hairline status.
        // This allows the user to avoid filling paths which should not be filled.
        bool       fHairline{false};
        // This is set if the path is significantly different from what a reasonable interpreter of
        // the underlying font data would produce. This is set if any non-identity matrix, stroke,
        // path effect, emboldening, etc is applied.
        // This allows Document implementations to know if a glyph should be drawn out of the font
        // data or needs to be embedded differently.
        bool       fModified{false};
    };

    struct DrawableData {
        Intercept* fIntercept{nullptr};
        sk_sp<SkDrawable> fDrawable;
        bool fHasDrawable{false};
    };

    size_t allocImage(SkArenaAlloc* alloc);

    void installImage(void* imageData) {
        SkASSERT(!this->setImageHasBeenCalled());
        fImage = imageData;
    }

    // path == nullptr indicates that there is no path.
    void installPath(SkArenaAlloc* alloc, const SkPath* path, bool hairline, bool modified);

    // drawable == nullptr indicates that there is no path.
    void installDrawable(SkArenaAlloc* alloc, sk_sp<SkDrawable> drawable);

    // The width and height of the glyph mask.
    uint16_t  fWidth  = 0,
              fHeight = 0;

    // The offset from the glyphs origin on the baseline to the top left of the glyph mask.
    int16_t   fTop  = 0,
              fLeft = 0;

    // fImage must remain null if the glyph is empty or if width > kMaxGlyphWidth.
    void*     fImage    = nullptr;

    // Path data has tricky state. If the glyph isEmpty, then fPathData should always be nullptr,
    // else if fPathData is not null, then a path has been requested. The fPath field of fPathData
    // may still be null after the request meaning that there is no path for this glyph.
    PathData* fPathData = nullptr;
    DrawableData* fDrawableData = nullptr;

    // The advance for this glyph.
    float     fAdvanceX = 0,
              fAdvanceY = 0;

    SkMask::Format fMaskFormat{SkMask::kBW_Format};

    // Used by the SkScalerContext to pass state from generateMetrics to generateImage.
    // Usually specifies which glyph representation was used to generate the metrics.
    uint16_t  fScalerContextBits = 0;

    // An SkGlyph can be created with just a packedID, but generally speaking some glyph factory
    // needs to actually fill out the glyph before it can be used as part of that system.
    SkDEBUGCODE(bool fAdvancesBoundsFormatAndInitialPathDone{false};)

    SkPackedGlyphID fID;
};

#endif
