/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStrike.h"

#include "SkGraphics.h"
#include "SkMakeUnique.h"
#include "SkMutex.h"
#include "SkOnce.h"
#include "SkPath.h"
#include "SkTemplates.h"
#include "SkTypeface.h"
#include <cctype>

namespace {
size_t compute_path_size(const SkPath& path) {
    return sizeof(SkPath) + path.countPoints() * sizeof(SkPoint);
}
}  // namespace

SkStrike::SkStrike(
    const SkDescriptor& desc,
    std::unique_ptr<SkScalerContext> scaler,
    const SkFontMetrics& fontMetrics)
    : fDesc{desc}
    , fScalerContext{std::move(scaler)}
    , fFontMetrics{fontMetrics}
    , fIsSubpixel{fScalerContext->isSubpixel()}
    , fAxisAlignment{fScalerContext->computeAxisAlignmentForHText()}
{
    SkASSERT(fScalerContext != nullptr);
    fMemoryUsed = sizeof(*this);
}

const SkDescriptor& SkStrike::getDescriptor() const {
    return *fDesc.getDesc();
}

#ifdef SK_DEBUG
#define VALIDATE()  AutoValidate av(this)
#else
#define VALIDATE()
#endif

unsigned SkStrike::getGlyphCount() const {
    return fScalerContext->getGlyphCount();
}

int SkStrike::countCachedGlyphs() const {
    return fGlyphMap.count();
}

bool SkStrike::isGlyphCached(SkGlyphID glyphID, SkFixed x, SkFixed y) const {
    SkPackedGlyphID packedGlyphID{glyphID, x, y};
    return fGlyphMap.find(packedGlyphID) != nullptr;
}

SkGlyph* SkStrike::getRawGlyphByID(SkPackedGlyphID id) {
    return lookupByPackedGlyphID(id, kNothing_MetricsType);
}

const SkGlyph& SkStrike::getGlyphIDAdvance(uint16_t glyphID) {
    VALIDATE();
    SkPackedGlyphID packedGlyphID(glyphID);
    return *this->lookupByPackedGlyphID(packedGlyphID, kJustAdvance_MetricsType);
}

const SkGlyph& SkStrike::getGlyphIDMetrics(uint16_t glyphID) {
    VALIDATE();
    SkPackedGlyphID packedGlyphID(glyphID);
    return *this->lookupByPackedGlyphID(packedGlyphID, kFull_MetricsType);
}

const SkGlyph& SkStrike::getGlyphIDMetrics(uint16_t glyphID, SkFixed x, SkFixed y) {
    VALIDATE();
    SkPackedGlyphID packedGlyphID(glyphID, x, y);
    return *this->lookupByPackedGlyphID(packedGlyphID, kFull_MetricsType);
}

void SkStrike::getAdvances(SkSpan<const SkGlyphID> glyphIDs, SkPoint advances[]) {
    for (auto glyphID : glyphIDs) {
        auto glyph = this->getGlyphIDAdvance(glyphID);
        *advances++ = SkPoint::Make(glyph.fAdvanceX, glyph.fAdvanceY);
    }
}

SkGlyph* SkStrike::lookupByPackedGlyphID(SkPackedGlyphID packedGlyphID, MetricsType type) {
    SkGlyph* glyphPtr = fGlyphMap.findOrNull(packedGlyphID);

    if (glyphPtr == nullptr) {
        // Glyph is not present in the stirke. Make a new glyph and fill it in.

        fMemoryUsed += sizeof(SkGlyph);
        glyphPtr = fAlloc.make<SkGlyph>(packedGlyphID);
        fGlyphMap.set(glyphPtr);

        switch (type) {
            // * Nothing - is only used for raw glyphs. It is assumed that the advances, etc. are
            // filled in by external code. This is used by the remote glyph cache to fill in glyphs.
            case kNothing_MetricsType:
                break;
            case kJustAdvance_MetricsType:
                fScalerContext->getAdvance(glyphPtr);
                break;
            case kFull_MetricsType:
                fScalerContext->getMetrics(glyphPtr);
                break;
        }
    } else {
        // Glyph is present in strike. Make sure the glyph has the right data.

        if (type == kFull_MetricsType && glyphPtr->isJustAdvance()) {
            fScalerContext->getMetrics(glyphPtr);
        }
    }

    return glyphPtr;
}

const void* SkStrike::findImage(const SkGlyph& glyph) {
    if (glyph.fWidth > 0 && glyph.fWidth < kMaxGlyphWidth) {
        if (nullptr == glyph.fImage) {
            SkDEBUGCODE(SkMask::Format oldFormat = (SkMask::Format)glyph.fMaskFormat);
            size_t  size = const_cast<SkGlyph&>(glyph).allocImage(&fAlloc);
            // check that alloc() actually succeeded
            if (glyph.fImage) {
                fScalerContext->getImage(glyph);
                // TODO: the scaler may have changed the maskformat during
                // getImage (e.g. from AA or LCD to BW) which means we may have
                // overallocated the buffer. Check if the new computedImageSize
                // is smaller, and if so, strink the alloc size in fImageAlloc.
                fMemoryUsed += size;
            }
            SkASSERT(oldFormat == glyph.fMaskFormat);
        }
    }
    return glyph.fImage;
}

void SkStrike::initializeImage(const volatile void* data, size_t size, SkGlyph* glyph) {
    SkASSERT(!glyph->fImage);

    if (glyph->fWidth > 0 && glyph->fWidth < kMaxGlyphWidth) {
        size_t allocSize = glyph->allocImage(&fAlloc);
        // check that alloc() actually succeeded
        if (glyph->fImage) {
            SkASSERT(size == allocSize);
            memcpy(glyph->fImage, const_cast<const void*>(data), allocSize);
            fMemoryUsed += size;
        }
    }
}

const SkPath* SkStrike::findPath(const SkGlyph& glyph) {

    if (!glyph.isEmpty()) {
        // If the path already exists, return it.
        if (glyph.fPathData != nullptr) {
            if (glyph.fPathData->fHasPath) {
                return &glyph.fPathData->fPath;
            }
            return nullptr;
        }

        const_cast<SkGlyph&>(glyph).addPath(fScalerContext.get(), &fAlloc);
        if (glyph.fPathData != nullptr) {
            fMemoryUsed += compute_path_size(glyph.fPathData->fPath);
        }

        return glyph.path();
    }

    return nullptr;
}

bool SkStrike::initializePath(SkGlyph* glyph, const volatile void* data, size_t size) {
    SkASSERT(!glyph->fPathData);

    if (glyph->fWidth) {
        SkGlyph::PathData* pathData = fAlloc.make<SkGlyph::PathData>();
        glyph->fPathData = pathData;
        if (size == 0u) return true;

        auto path = skstd::make_unique<SkPath>();
        if (!pathData->fPath.readFromMemory(const_cast<const void*>(data), size)) {
            return false;
        }
        fMemoryUsed += compute_path_size(glyph->fPathData->fPath);
        pathData->fHasPath = true;
    }

    return true;
}

bool SkStrike::belongsToCache(const SkGlyph* glyph) const {
    return glyph && fGlyphMap.findOrNull(glyph->getPackedID()) == glyph;
}

const SkGlyph* SkStrike::getCachedGlyphAnySubPix(SkGlyphID glyphID,
                                                     SkPackedGlyphID vetoID) const {
    for (SkFixed subY = 0; subY < SK_Fixed1; subY += SK_FixedQuarter) {
        for (SkFixed subX = 0; subX < SK_Fixed1; subX += SK_FixedQuarter) {
            SkPackedGlyphID packedGlyphID{glyphID, subX, subY};
            if (packedGlyphID == vetoID) continue;
            if (SkGlyph* glyphPtr = fGlyphMap.findOrNull(packedGlyphID)) {
                return glyphPtr;
            }
        }
    }

    return nullptr;
}

void SkStrike::initializeGlyphFromFallback(SkGlyph* glyph, const SkGlyph& fallback) {
    fMemoryUsed += glyph->copyImageData(fallback, &fAlloc);
}

SkVector SkStrike::rounding() const {
    return SkStrikeCommon::PixelRounding(fIsSubpixel, fAxisAlignment);
}

const SkGlyph& SkStrike::getGlyphMetrics(SkGlyphID glyphID, SkPoint position) {
    if (!fIsSubpixel) {
        return this->getGlyphIDMetrics(glyphID);
    } else {
        SkIPoint lookupPosition = SkStrikeCommon::SubpixelLookup(fAxisAlignment, position);

        return this->getGlyphIDMetrics(glyphID, lookupPosition.x(), lookupPosition.y());
    }
}

// N.B. This glyphMetrics call culls all the glyphs which will not display based on a non-finite
// position or that there are no mask pixels.
SkSpan<const SkGlyphPos> SkStrike::prepareForDrawing(const SkGlyphID glyphIDs[],
                                                     const SkPoint positions[],
                                                     size_t n,
                                                     int maxDimension,
                                                     SkGlyphPos result[]) {
    size_t drawableGlyphCount = 0;
    for (size_t i = 0; i < n; i++) {
        SkPoint position = positions[i];
        if (SkScalarsAreFinite(position.x(), position.y())) {
            // This assumes that the strike has no sub-pixel positioning for glyphs that are
            // transformed from source space to device space.
            const SkGlyph& glyph = this->getGlyphMetrics(glyphIDs[i], position);
            if (!glyph.isEmpty()) {
                result[drawableGlyphCount++] = {i, &glyph, position};
                if (glyph.maxDimension() <= maxDimension) {
                    // Glyph fits in the atlas, good to go.
                    this->findImage(glyph);
                } else if (glyph.fMaskFormat != SkMask::kARGB32_Format) {
                    // The out of atlas glyph is not color so we can draw it using paths.
                    this->findPath(glyph);
                } else {
                    // This will be handled by the fallback strike.
                    SkASSERT(glyph.maxDimension() > maxDimension
                             && glyph.fMaskFormat == SkMask::kARGB32_Format);
                }
            }
        }
    }

    return SkSpan<const SkGlyphPos>{result, drawableGlyphCount};
}

#include "../pathops/SkPathOpsCubic.h"
#include "../pathops/SkPathOpsQuad.h"

static bool quad_in_bounds(const SkScalar* pts, const SkScalar bounds[2]) {
    SkScalar min = SkTMin(SkTMin(pts[0], pts[2]), pts[4]);
    if (bounds[1] < min) {
        return false;
    }
    SkScalar max = SkTMax(SkTMax(pts[0], pts[2]), pts[4]);
    return bounds[0] < max;
}

static bool cubic_in_bounds(const SkScalar* pts, const SkScalar bounds[2]) {
    SkScalar min = SkTMin(SkTMin(SkTMin(pts[0], pts[2]), pts[4]), pts[6]);
    if (bounds[1] < min) {
        return false;
    }
    SkScalar max = SkTMax(SkTMax(SkTMax(pts[0], pts[2]), pts[4]), pts[6]);
    return bounds[0] < max;
}

void SkStrike::OffsetResults(const SkGlyph::Intercept* intercept, SkScalar scale,
                                 SkScalar xPos, SkScalar* array, int* count) {
    if (array) {
        array += *count;
        for (int index = 0; index < 2; index++) {
            *array++ = intercept->fInterval[index] * scale + xPos;
        }
    }
    *count += 2;
}

void SkStrike::AddInterval(SkScalar val, SkGlyph::Intercept* intercept) {
    intercept->fInterval[0] = SkTMin(intercept->fInterval[0], val);
    intercept->fInterval[1] = SkTMax(intercept->fInterval[1], val);
}

void SkStrike::AddPoints(const SkPoint* pts, int ptCount, const SkScalar bounds[2],
        bool yAxis, SkGlyph::Intercept* intercept) {
    for (int i = 0; i < ptCount; ++i) {
        SkScalar val = *(&pts[i].fY - yAxis);
        if (bounds[0] < val && val < bounds[1]) {
            AddInterval(*(&pts[i].fX + yAxis), intercept);
        }
    }
}

void SkStrike::AddLine(const SkPoint pts[2], SkScalar axis, bool yAxis,
                           SkGlyph::Intercept* intercept) {
    SkScalar t = yAxis ? sk_ieee_float_divide(axis - pts[0].fX, pts[1].fX - pts[0].fX)
                       : sk_ieee_float_divide(axis - pts[0].fY, pts[1].fY - pts[0].fY);
    if (0 <= t && t < 1) {   // this handles divide by zero above
        AddInterval(yAxis ? pts[0].fY + t * (pts[1].fY - pts[0].fY)
            : pts[0].fX + t * (pts[1].fX - pts[0].fX), intercept);
    }
}

void SkStrike::AddQuad(const SkPoint pts[3], SkScalar axis, bool yAxis,
                     SkGlyph::Intercept* intercept) {
    SkDQuad quad;
    quad.set(pts);
    double roots[2];
    int count = yAxis ? quad.verticalIntersect(axis, roots)
            : quad.horizontalIntersect(axis, roots);
    while (--count >= 0) {
        SkPoint pt = quad.ptAtT(roots[count]).asSkPoint();
        AddInterval(*(&pt.fX + yAxis), intercept);
    }
}

void SkStrike::AddCubic(const SkPoint pts[4], SkScalar axis, bool yAxis,
                      SkGlyph::Intercept* intercept) {
    SkDCubic cubic;
    cubic.set(pts);
    double roots[3];
    int count = yAxis ? cubic.verticalIntersect(axis, roots)
            : cubic.horizontalIntersect(axis, roots);
    while (--count >= 0) {
        SkPoint pt = cubic.ptAtT(roots[count]).asSkPoint();
        AddInterval(*(&pt.fX + yAxis), intercept);
    }
}

const SkGlyph::Intercept* SkStrike::MatchBounds(const SkGlyph* glyph,
                                                    const SkScalar bounds[2]) {
    if (!glyph->fPathData) {
        return nullptr;
    }
    const SkGlyph::Intercept* intercept = glyph->fPathData->fIntercept;
    while (intercept) {
        if (bounds[0] == intercept->fBounds[0] && bounds[1] == intercept->fBounds[1]) {
            return intercept;
        }
        intercept = intercept->fNext;
    }
    return nullptr;
}

void SkStrike::findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
        bool yAxis, SkGlyph* glyph, SkScalar* array, int* count) {
    const SkGlyph::Intercept* match = MatchBounds(glyph, bounds);

    if (match) {
        if (match->fInterval[0] < match->fInterval[1]) {
            OffsetResults(match, scale, xPos, array, count);
        }
        return;
    }

    SkGlyph::Intercept* intercept = fAlloc.make<SkGlyph::Intercept>();
    intercept->fNext = glyph->fPathData->fIntercept;
    intercept->fBounds[0] = bounds[0];
    intercept->fBounds[1] = bounds[1];
    intercept->fInterval[0] = SK_ScalarMax;
    intercept->fInterval[1] = SK_ScalarMin;
    glyph->fPathData->fIntercept = intercept;
    const SkPath* path = &(glyph->fPathData->fPath);
    const SkRect& pathBounds = path->getBounds();
    if (*(&pathBounds.fBottom - yAxis) < bounds[0] || bounds[1] < *(&pathBounds.fTop - yAxis)) {
        return;
    }
    SkPath::Iter iter(*path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while (SkPath::kDone_Verb != (verb = iter.next(pts))) {
        switch (verb) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
                AddLine(pts, bounds[0], yAxis, intercept);
                AddLine(pts, bounds[1], yAxis, intercept);
                AddPoints(pts, 2, bounds, yAxis, intercept);
                break;
            case SkPath::kQuad_Verb:
                if (!quad_in_bounds(&pts[0].fY - yAxis, bounds)) {
                    break;
                }
                AddQuad(pts, bounds[0], yAxis, intercept);
                AddQuad(pts, bounds[1], yAxis, intercept);
                AddPoints(pts, 3, bounds, yAxis, intercept);
                break;
            case SkPath::kConic_Verb:
                SkASSERT(0);  // no support for text composed of conics
                break;
            case SkPath::kCubic_Verb:
                if (!cubic_in_bounds(&pts[0].fY - yAxis, bounds)) {
                    break;
                }
                AddCubic(pts, bounds[0], yAxis, intercept);
                AddCubic(pts, bounds[1], yAxis, intercept);
                AddPoints(pts, 4, bounds, yAxis, intercept);
                break;
            case SkPath::kClose_Verb:
                break;
            default:
                SkASSERT(0);
                break;
        }
    }
    if (intercept->fInterval[0] >= intercept->fInterval[1]) {
        intercept->fInterval[0] = SK_ScalarMax;
        intercept->fInterval[1] = SK_ScalarMin;
        return;
    }
    OffsetResults(intercept, scale, xPos, array, count);
}

void SkStrike::dump() const {
    const SkTypeface* face = fScalerContext->getTypeface();
    const SkScalerContextRec& rec = fScalerContext->getRec();
    SkMatrix matrix;
    rec.getSingleMatrix(&matrix);
    matrix.preScale(SkScalarInvert(rec.fTextSize), SkScalarInvert(rec.fTextSize));
    SkString name;
    face->getFamilyName(&name);

    SkString msg;
    SkFontStyle style = face->fontStyle();
    msg.printf("cache typeface:%x %25s:(%d,%d,%d)\n %s glyphs:%3d",
               face->uniqueID(), name.c_str(), style.weight(), style.width(), style.slant(),
               rec.dump().c_str(), fGlyphMap.count());
    SkDebugf("%s\n", msg.c_str());
}

void SkStrike::generatePath(const SkGlyph& glyph) {
    if (!glyph.isEmpty()) { this->findPath(glyph); }
}

void SkStrike::onAboutToExitScope() { }

#ifdef SK_DEBUG
void SkStrike::forceValidate() const {
    size_t memoryUsed = sizeof(*this);
    fGlyphMap.foreach ([&memoryUsed](const SkGlyph* glyphPtr) {
        memoryUsed += sizeof(SkGlyph);
        if (glyphPtr->fImage) {
            memoryUsed += glyphPtr->computeImageSize();
        }
        if (glyphPtr->fPathData) {
            memoryUsed += compute_path_size(glyphPtr->fPathData->fPath);
        }
    });
    SkASSERT(fMemoryUsed == memoryUsed);
}

void SkStrike::validate() const {
#ifdef SK_DEBUG_GLYPH_CACHE
    forceValidate();
#endif
}
#endif  // SK_DEBUG


