/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyph.h"

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkScalerContext.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsQuad.h"

SkMask SkGlyph::mask() const {
    // getMetrics had to be called.
    SkASSERT(fMaskFormat != MASK_FORMAT_UNKNOWN);

    SkMask mask;
    mask.fImage = (uint8_t*)fImage;
    mask.fBounds.setXYWH(fLeft, fTop, fWidth, fHeight);
    mask.fRowBytes = this->rowBytes();
    mask.fFormat = static_cast<SkMask::Format>(fMaskFormat);
    return mask;
}

SkMask SkGlyph::mask(SkPoint position) const {
    SkMask answer = this->mask();
    answer.fBounds.offset(SkScalarFloorToInt(position.x()), SkScalarFloorToInt(position.y()));
    return answer;
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

SkGlyph::SkGlyph(const SkGlyphPrototype& p)
    : fWidth{p.width}
    , fHeight{p.height}
    , fTop{p.top}
    , fLeft{p.left}
    , fAdvanceX{p.advanceX}
    , fAdvanceY{p.advanceY}
    , fMaskFormat{(uint8_t)p.maskFormat}
    , fForceBW{p.forceBW}
    , fID{p.id}
    {}

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

bool SkGlyph::setMetricsAndImage(SkArenaAlloc* alloc, const SkGlyph& from) {
    if (fImage == nullptr) {
        fAdvanceX = from.fAdvanceX;
        fAdvanceY = from.fAdvanceY;
        fWidth = from.fWidth;
        fHeight = from.fHeight;
        fTop = from.fTop;
        fLeft = from.fLeft;
        fForceBW = from.fForceBW;
        fMaskFormat = from.fMaskFormat;
        return this->setImage(alloc, from.image());
    }
    return false;
}

size_t SkGlyph::rowBytes() const {
    return format_rowbytes(fWidth, (SkMask::Format)fMaskFormat);
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

void SkGlyph::installPath(SkArenaAlloc* alloc, const SkPath* path) {
    SkASSERT(fPathData == nullptr);
    SkASSERT(!this->setPathHasBeenCalled());
    fPathData = alloc->make<SkGlyph::PathData>();
    if (path != nullptr) {
        fPathData->fPath = *path;
        fPathData->fPath.updateBoundsCache();
        fPathData->fPath.getGenerationID();
        fPathData->fHasPath = true;
    }
}

bool SkGlyph::setPath(SkArenaAlloc* alloc, SkScalerContext* scalerContext) {
    if (!this->setPathHasBeenCalled()) {
        SkPath path;
        if (scalerContext->getPath(this->getPackedID(), &path)) {
            this->installPath(alloc, &path);
        } else {
            this->installPath(alloc, nullptr);
        }
        return this->path() != nullptr;
    }

    return false;
}

bool SkGlyph::setPath(SkArenaAlloc* alloc, const SkPath* path) {
    if (!this->setPathHasBeenCalled()) {
        this->installPath(alloc, path);
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

static std::tuple<SkScalar, SkScalar> calculate_path_gap(
        SkScalar topOffset, SkScalar bottomOffset, const SkPath& path) {

    // Left and Right of an ever expanding gap around the path.
    SkScalar left  = SK_ScalarMax,
             right = SK_ScalarMin;
    auto expandGap = [&left, &right](SkScalar v) {
        left  = SkTMin(left, v);
        right = SkTMax(right, v);
    };

    // Handle all the different verbs for the path.
    SkPoint pts[4];
    auto addLine = [&expandGap, &pts](SkScalar offset) {
        SkScalar t = sk_ieee_float_divide(offset - pts[0].fY, pts[1].fY - pts[0].fY);
        if (0 <= t && t < 1) {   // this handles divide by zero above
            expandGap(pts[0].fX + t * (pts[1].fX - pts[0].fX));
        }
    };

    auto addQuad = [&expandGap, &pts](SkScalar offset) {
        SkDQuad quad;
        quad.set(pts);
        double roots[2];
        int count = quad.horizontalIntersect(offset, roots);
        while (--count >= 0) {
            expandGap(quad.ptAtT(roots[count]).asSkPoint().fX);
        }
    };

    auto addCubic = [&expandGap, &pts](SkScalar offset) {
        SkDCubic cubic;
        cubic.set(pts);
        double roots[3];
        int count = cubic.horizontalIntersect(offset, roots);
        while (--count >= 0) {
            expandGap(cubic.ptAtT(roots[count]).asSkPoint().fX);
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
                addLine(topOffset);
                addLine(bottomOffset);
                addPts(2);
                break;
            }
            case SkPath::kQuad_Verb: {
                SkScalar quadTop = SkTMin(SkTMin(pts[0].fY, pts[1].fY), pts[2].fY);
                if (bottomOffset < quadTop) { break; }
                SkScalar quadBottom = SkTMax(SkTMax(pts[0].fY, pts[1].fY), pts[2].fY);
                if (topOffset > quadBottom) { break; }
                addQuad(topOffset);
                addQuad(bottomOffset);
                addPts(3);
                break;
            }
            case SkPath::kConic_Verb: {
                SkASSERT(0);  // no support for text composed of conics
                break;
            }
            case SkPath::kCubic_Verb: {
                SkScalar quadTop =
                        SkTMin(SkTMin(SkTMin(pts[0].fY, pts[1].fY), pts[2].fY), pts[3].fY);
                if (bottomOffset < quadTop) { break; }
                SkScalar quadBottom =
                        SkTMax(SkTMax(SkTMax(pts[0].fY, pts[1].fY), pts[2].fY), pts[3].fY);
                if (topOffset > quadBottom) { break; }
                addCubic(topOffset);
                addCubic(bottomOffset);
                addPts(4);
                break;
            }
            case SkPath::kClose_Verb: {
                break;
            }
            default: {
                SkASSERT(0);
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
                if (!fPathData) {
                    return nullptr;
                }
                const SkGlyph::Intercept* intercept = fPathData->fIntercept;
                while (intercept) {
                    if (bounds[0] == intercept->fBounds[0] && bounds[1] == intercept->fBounds[1]) {
                        return intercept;
                    }
                    intercept = intercept->fNext;
                }
                return nullptr;
            }(bounds);

    if (match) {
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
