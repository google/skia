/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphCache.h"

#include "SkGraphics.h"
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

SkGlyphCache::SkGlyphCache(
    const SkDescriptor& desc,
    std::unique_ptr<SkScalerContext> scaler,
    const SkPaint::FontMetrics& fontMetrics)
    : fDesc{desc}
    , fScalerContext{std::move(scaler)}
    , fFontMetrics(fontMetrics)
{
    SkASSERT(fScalerContext != nullptr);
    fMemoryUsed = sizeof(*this);
}

SkGlyphCache::~SkGlyphCache() {
    fGlyphMap.foreach([](SkGlyph* g) {
        if (g->fPathData) {
            delete g->fPathData->fPath;
        }
    });
}

const SkDescriptor& SkGlyphCache::getDescriptor() const {
    return *fDesc.getDesc();
}

SkGlyphCache::CharGlyphRec* SkGlyphCache::getCharGlyphRec(SkPackedUnicharID packedUnicharID) {
    if (!fPackedUnicharIDToPackedGlyphID) {
        fPackedUnicharIDToPackedGlyphID.reset(new CharGlyphRec[kHashCount]);
    }

    return &fPackedUnicharIDToPackedGlyphID[packedUnicharID.hash() & kHashMask];
}

#ifdef SK_DEBUG
#define VALIDATE()  AutoValidate av(this)
#else
#define VALIDATE()
#endif

SkGlyphID SkGlyphCache::unicharToGlyph(SkUnichar charCode) {
    VALIDATE();
    SkPackedUnicharID packedUnicharID(charCode);
    CharGlyphRec* rec = this->getCharGlyphRec(packedUnicharID);

    if (rec->fPackedUnicharID == packedUnicharID) {
        // The glyph exists in the unichar to glyph mapping cache. Return it.
        return rec->fPackedGlyphID.code();
    } else {
        // The glyph is not in the unichar to glyph mapping cache. Insert it.
        rec->fPackedUnicharID = packedUnicharID;
        SkGlyphID glyphID = fScalerContext->charToGlyphID(charCode);
        rec->fPackedGlyphID = SkPackedGlyphID(glyphID);
        return glyphID;
    }
}

SkUnichar SkGlyphCache::glyphToUnichar(SkGlyphID glyphID) {
    return fScalerContext->glyphIDToChar(glyphID);
}

unsigned SkGlyphCache::getGlyphCount() const {
    return fScalerContext->getGlyphCount();
}

int SkGlyphCache::countCachedGlyphs() const {
    return fGlyphMap.count();
}

bool SkGlyphCache::isGlyphCached(SkGlyphID glyphID, SkFixed x, SkFixed y) const {
    SkPackedGlyphID packedGlyphID{glyphID, x, y};
    return fGlyphMap.find(packedGlyphID) != nullptr;
}

SkGlyph* SkGlyphCache::getRawGlyphByID(SkPackedGlyphID id) {
    return lookupByPackedGlyphID(id, kNothing_MetricsType);
}

const SkGlyph& SkGlyphCache::getUnicharAdvance(SkUnichar charCode) {
    VALIDATE();
    return *this->lookupByChar(charCode, kJustAdvance_MetricsType);
}

const SkGlyph& SkGlyphCache::getGlyphIDAdvance(uint16_t glyphID) {
    VALIDATE();
    SkPackedGlyphID packedGlyphID(glyphID);
    return *this->lookupByPackedGlyphID(packedGlyphID, kJustAdvance_MetricsType);
}

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode) {
    VALIDATE();
    return *this->lookupByChar(charCode, kFull_MetricsType);
}

const SkGlyph& SkGlyphCache::getUnicharMetrics(SkUnichar charCode, SkFixed x, SkFixed y) {
    VALIDATE();
    return *this->lookupByChar(charCode, kFull_MetricsType, x, y);
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID) {
    VALIDATE();
    SkPackedGlyphID packedGlyphID(glyphID);
    return *this->lookupByPackedGlyphID(packedGlyphID, kFull_MetricsType);
}

const SkGlyph& SkGlyphCache::getGlyphIDMetrics(uint16_t glyphID, SkFixed x, SkFixed y) {
    VALIDATE();
    SkPackedGlyphID packedGlyphID(glyphID, x, y);
    return *this->lookupByPackedGlyphID(packedGlyphID, kFull_MetricsType);
}

void SkGlyphCache::getAdvances(SkSpan<const SkGlyphID> glyphIDs, SkPoint advances[]) {
    for (auto glyphID : glyphIDs) {
        auto glyph = this->getGlyphIDAdvance(glyphID);
        *advances++ = SkPoint::Make(glyph.fAdvanceX, glyph.fAdvanceY);
    }
}

SkGlyph* SkGlyphCache::lookupByChar(SkUnichar charCode, MetricsType type, SkFixed x, SkFixed y) {
    SkPackedUnicharID id(charCode, x, y);
    CharGlyphRec* rec = this->getCharGlyphRec(id);
    if (rec->fPackedUnicharID != id) {
        rec->fPackedUnicharID = id;
        rec->fPackedGlyphID = SkPackedGlyphID(fScalerContext->charToGlyphID(charCode), x, y);
    }
    return this->lookupByPackedGlyphID(rec->fPackedGlyphID, type);
}

SkGlyph* SkGlyphCache::lookupByPackedGlyphID(SkPackedGlyphID packedGlyphID, MetricsType type) {
    SkGlyph* glyph = fGlyphMap.find(packedGlyphID);

    if (nullptr == glyph) {
        glyph = this->allocateNewGlyph(packedGlyphID, type);
    } else {
        if (type == kFull_MetricsType && glyph->isJustAdvance()) {
            fScalerContext->getMetrics(glyph);
        }
    }
    return glyph;
}

SkGlyph* SkGlyphCache::allocateNewGlyph(SkPackedGlyphID packedGlyphID, MetricsType mtype) {
    fMemoryUsed += sizeof(SkGlyph);

    SkGlyph* glyphPtr;
    {
        SkGlyph glyph;
        glyph.initWithGlyphID(packedGlyphID);
        glyphPtr = fGlyphMap.set(glyph);
    }

    if (kNothing_MetricsType == mtype) {
        return glyphPtr;
    } else if (kJustAdvance_MetricsType == mtype) {
        fScalerContext->getAdvance(glyphPtr);
    } else {
        SkASSERT(kFull_MetricsType == mtype);
        fScalerContext->getMetrics(glyphPtr);
    }

    SkASSERT(glyphPtr->fID != SkPackedGlyphID());
    return glyphPtr;
}

const void* SkGlyphCache::findImage(const SkGlyph& glyph) {
    if (glyph.fWidth > 0 && glyph.fWidth < kMaxGlyphWidth) {
        if (nullptr == glyph.fImage) {
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
        }
    }
    return glyph.fImage;
}

void SkGlyphCache::initializeImage(const volatile void* data, size_t size, SkGlyph* glyph) {
    // Don't overwrite the image if we already have one. We could have used a fallback if the
    // glyph was missing earlier.
    if (glyph->fImage) return;

    if (glyph->fWidth > 0 && glyph->fWidth < kMaxGlyphWidth) {
        size_t allocSize = glyph->allocImage(&fAlloc);
        // check that alloc() actually succeeded
        if (glyph->fImage) {
            SkAssertResult(size == allocSize);
            memcpy(glyph->fImage, const_cast<const void*>(data), allocSize);
            fMemoryUsed += size;
        }
    }
}

const SkPath* SkGlyphCache::findPath(const SkGlyph& glyph) {
    if (glyph.fWidth) {
        if (glyph.fPathData == nullptr) {
            SkGlyph::PathData* pathData = fAlloc.make<SkGlyph::PathData>();
            const_cast<SkGlyph&>(glyph).fPathData = pathData;
            pathData->fIntercept = nullptr;
            SkPath* path = new SkPath;
            if (fScalerContext->getPath(glyph.getPackedID(), path)) {
                path->updateBoundsCache();
                path->getGenerationID();
                pathData->fPath = path;
                fMemoryUsed += compute_path_size(*path);
            } else {
                pathData->fPath = nullptr;
                delete path;
            }
        }
    }
    return glyph.fPathData ? glyph.fPathData->fPath : nullptr;
}

bool SkGlyphCache::initializePath(SkGlyph* glyph, const volatile void* data, size_t size) {
    // Don't overwrite the path if we already have one. We could have used a fallback if the
    // glyph was missing earlier.
    if (glyph->fPathData) return true;

    if (glyph->fWidth) {
        SkGlyph::PathData* pathData = fAlloc.make<SkGlyph::PathData>();
        glyph->fPathData = pathData;
        pathData->fIntercept = nullptr;
        SkPath* path = new SkPath;
        if (!path->readFromMemory(const_cast<const void*>(data), size)) {
            delete path;
            return false;
        }
        pathData->fPath = path;
        fMemoryUsed += compute_path_size(*path);
    }

    return true;
}

bool SkGlyphCache::belongsToCache(const SkGlyph* glyph) const {
    return glyph && fGlyphMap.find(glyph->getPackedID()) == glyph;
}

const SkGlyph* SkGlyphCache::getCachedGlyphAnySubPix(SkGlyphID glyphID,
                                                     SkPackedGlyphID vetoID) const {
    for (SkFixed subY = 0; subY < SK_Fixed1; subY += SK_FixedQuarter) {
        for (SkFixed subX = 0; subX < SK_Fixed1; subX += SK_FixedQuarter) {
            SkPackedGlyphID packedGlyphID{glyphID, subX, subY};
            if (packedGlyphID == vetoID) continue;
            if (const auto* glyph = fGlyphMap.find(packedGlyphID)) {
                return glyph;
            }
        }
    }

    return nullptr;
}

void SkGlyphCache::initializeGlyphFromFallback(SkGlyph* glyph, const SkGlyph& fallback) {
    fMemoryUsed += glyph->copyImageData(fallback, &fAlloc);
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

void SkGlyphCache::OffsetResults(const SkGlyph::Intercept* intercept, SkScalar scale,
                                 SkScalar xPos, SkScalar* array, int* count) {
    if (array) {
        array += *count;
        for (int index = 0; index < 2; index++) {
            *array++ = intercept->fInterval[index] * scale + xPos;
        }
    }
    *count += 2;
}

void SkGlyphCache::AddInterval(SkScalar val, SkGlyph::Intercept* intercept) {
    intercept->fInterval[0] = SkTMin(intercept->fInterval[0], val);
    intercept->fInterval[1] = SkTMax(intercept->fInterval[1], val);
}

void SkGlyphCache::AddPoints(const SkPoint* pts, int ptCount, const SkScalar bounds[2],
        bool yAxis, SkGlyph::Intercept* intercept) {
    for (int i = 0; i < ptCount; ++i) {
        SkScalar val = *(&pts[i].fY - yAxis);
        if (bounds[0] < val && val < bounds[1]) {
            AddInterval(*(&pts[i].fX + yAxis), intercept);
        }
    }
}

void SkGlyphCache::AddLine(const SkPoint pts[2], SkScalar axis, bool yAxis,
                     SkGlyph::Intercept* intercept) {
    SkScalar t = yAxis ? (axis - pts[0].fX) / (pts[1].fX - pts[0].fX)
            : (axis - pts[0].fY) / (pts[1].fY - pts[0].fY);
    if (0 <= t && t < 1) {   // this handles divide by zero above
        AddInterval(yAxis ? pts[0].fY + t * (pts[1].fY - pts[0].fY)
            : pts[0].fX + t * (pts[1].fX - pts[0].fX), intercept);
    }
}

void SkGlyphCache::AddQuad(const SkPoint pts[3], SkScalar axis, bool yAxis,
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

void SkGlyphCache::AddCubic(const SkPoint pts[4], SkScalar axis, bool yAxis,
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

const SkGlyph::Intercept* SkGlyphCache::MatchBounds(const SkGlyph* glyph,
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

void SkGlyphCache::findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
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
    const SkPath* path = glyph->fPathData->fPath;
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

void SkGlyphCache::dump() const {
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

#ifdef SK_DEBUG
void SkGlyphCache::forceValidate() const {
    size_t memoryUsed = sizeof(*this);
    fGlyphMap.foreach ([&memoryUsed](const SkGlyph& glyph) {
        memoryUsed += sizeof(SkGlyph);
        if (glyph.fImage) {
            memoryUsed += glyph.computeImageSize();
        }
        if (glyph.fPathData && glyph.fPathData->fPath) {
            memoryUsed += compute_path_size(*glyph.fPathData->fPath);
        }
    });
    SkASSERT(fMemoryUsed == memoryUsed);
}

void SkGlyphCache::validate() const {
#ifdef SK_DEBUG_GLYPH_CACHE
    forceValidate();
#endif
}

#endif


