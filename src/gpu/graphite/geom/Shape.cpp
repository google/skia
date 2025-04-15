/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/Shape.h"

#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"

#include <cstring>

namespace {
// Keys for paths may be extracted from the path data for small paths, to maximize matches
// even when the genIDs may differ. The value is based on emperical experience, to trade off
// matches vs. key size.
constexpr int kMaxKeyFromDataVerbCnt = 10;
}

namespace skgpu::graphite {

Shape& Shape::operator=(const Shape& shape) {
    switch (shape.type()) {
        case Type::kEmpty: this->reset();                         break;
        case Type::kLine:  this->setLine(shape.p0(), shape.p1()); break;
        case Type::kRect:  this->setRect(shape.rect());           break;
        case Type::kRRect: this->setRRect(shape.rrect());         break;
        case Type::kArc:   this->setArc(shape.arc());             break;
        case Type::kPath:  this->setPath(shape.path());           break;
    }

    fInverted = shape.fInverted;
    return *this;
}

bool Shape::conservativeContains(const Rect& rect) const {
    switch (fType) {
        case Type::kEmpty: return false;
        case Type::kLine:  return false;
        case Type::kRect:  return fRect.contains(rect);
        case Type::kRRect: return fRRect.contains(rect.asSkRect());
        case Type::kPath:  // We need to ensure the path is non-inverted.
                           if (this->inverted()) {
                               SkPath nonInverted(fPath);
                               nonInverted.toggleInverseFillType();
                               return nonInverted.conservativelyContainsRect(rect.asSkRect());
                           } else {
                               return fPath.conservativelyContainsRect(rect.asSkRect());
                           }
        case Type::kArc:   if (fArc.fType == SkArc::Type::kWedge) {
                               SkPath arc = this->asPath();
                               if (this->inverted()) {
                                   arc.toggleInverseFillType();
                               }
                               return arc.conservativelyContainsRect(rect.asSkRect());
                           } else {
                               return false;
                           }
    }
    SkUNREACHABLE;
}

bool Shape::conservativeContains(skvx::float2 point) const {
    switch (fType) {
        case Type::kEmpty: return false;
        case Type::kLine:  return false;
        case Type::kRect:  return fRect.contains(Rect::Point(point));
        case Type::kRRect: return SkRRectPriv::ContainsPoint(fRRect, {point.x(), point.y()});
        case Type::kPath:  // We need to ensure the path is non-inverted.
                           if (this->inverted()) {
                               SkPath nonInverted(fPath);
                               nonInverted.toggleInverseFillType();
                               return nonInverted.contains(point.x(), point.y());
                           } else {
                               return fPath.contains(point.x(), point.y());
                           }
        case Type::kArc:   return false;
    }
    SkUNREACHABLE;
}

bool Shape::convex(bool simpleFill) const {
    if (this->isPath()) {
        // SkPath.isConvex() really means "is this path convex were it to be closed".
        return (simpleFill || fPath.isLastContourClosed()) && fPath.isConvex();
    } else if (this->isArc()) {
        return SkPathPriv::DrawArcIsConvex(fArc.sweepAngle(), fArc.fType, simpleFill);
    } else {
        // Every other shape type is convex by construction.
        return true;
    }
}

Rect Shape::bounds() const {
    switch (fType) {
        case Type::kEmpty: return Rect(0, 0, 0, 0);
        case Type::kLine:  return fRect.makeSorted(); // sorting corners computes bbox of segment
        case Type::kRect:  return fRect; // assuming it's sorted
        case Type::kRRect: return fRRect.getBounds();
        case Type::kArc:   return fArc.oval();
        case Type::kPath:  return fPath.getBounds();
    }
    SkUNREACHABLE;
}

SkPath Shape::asPath() const {
    if (fType == Type::kPath) {
        return fPath;
    }

    if (fType == Type::kArc) {
        SkPath out;
        // Filled ovals are already culled out so we assume no simple fills
        SkPathPriv::CreateDrawArcPath(&out, fArc, /*isFillNoPathEffect=*/false);
        // CreateDrawArcPath resets the output path and configures its fill
        // type, so we just have to ensure invertedness is correct.
        if (fInverted) {
            out.toggleInverseFillType();
        }
        return out;
    }

    SkPathBuilder builder(this->fillType());
    switch (fType) {
        case Type::kEmpty: /* do nothing */                            break;
        case Type::kLine:  builder.moveTo(fRect.left(), fRect.top())
                                  .lineTo(fRect.right(), fRect.bot()); break;
        case Type::kRect:  builder.addRect(fRect.asSkRect());          break;
        case Type::kRRect: builder.addRRect(fRRect);                   break;
        case Type::kPath:
        case Type::kArc:   SkUNREACHABLE;
    }
    return builder.detach();
}

namespace {
int path_key_from_data_size(const SkPath& path) {
    const int verbCnt = path.countVerbs();
    if (verbCnt > kMaxKeyFromDataVerbCnt) {
        return -1;
    }
    const int pointCnt = path.countPoints();
    const int conicWeightCnt = SkPathPriv::ConicWeightCnt(path);

    static_assert(sizeof(SkPoint) == 2 * sizeof(uint32_t));
    static_assert(sizeof(SkScalar) == sizeof(uint32_t));
    // 1 is for the verb count. Each verb is a byte but we'll pad the verb data out to
    // a uint32_t length.
    return 1 + (SkAlign4(verbCnt) >> 2) + 2 * pointCnt + conicWeightCnt;
}

// Writes the path data key into the passed pointer.
void write_path_key_from_data(const SkPath& path, uint32_t* origKey) {
    uint32_t* key = origKey;
    // The check below should take care of negative values casted positive.
    const int verbCnt = path.countVerbs();
    const int pointCnt = path.countPoints();
    const int conicWeightCnt = SkPathPriv::ConicWeightCnt(path);
    SkASSERT(verbCnt <= kMaxKeyFromDataVerbCnt);
    SkASSERT(pointCnt && verbCnt);
    *key++ = verbCnt;
    memcpy(key, SkPathPriv::VerbData(path), verbCnt * sizeof(uint8_t));
    int verbKeySize = SkAlign4(verbCnt);
    // pad out to uint32_t alignment using value that will stand out when debugging.
    uint8_t* pad = reinterpret_cast<uint8_t*>(key)+ verbCnt;
    memset(pad, 0xDE, verbKeySize - verbCnt);
    key += verbKeySize >> 2;

    memcpy(key, SkPathPriv::PointData(path), sizeof(SkPoint) * pointCnt);
    static_assert(sizeof(SkPoint) == 2 * sizeof(uint32_t));
    key += 2 * pointCnt;
    sk_careful_memcpy(key, SkPathPriv::ConicWeightData(path), sizeof(SkScalar) * conicWeightCnt);
    static_assert(sizeof(SkScalar) == sizeof(uint32_t));
    SkDEBUGCODE(key += conicWeightCnt);
    SkASSERT(key - origKey == path_key_from_data_size(path));
}
} // anonymous namespace

int Shape::keySize() const {
    int count = 1; // Every key has the state flags from the Shape
    switch(this->type()) {
        case Type::kLine:
            static_assert(0 == sizeof(skvx::float4) % sizeof(uint32_t));
            count += sizeof(skvx::float4) / sizeof(uint32_t);
            break;
        case Type::kRect:
            static_assert(0 == sizeof(Rect) % sizeof(uint32_t));
            count += sizeof(Rect) / sizeof(uint32_t);
            break;
        case Type::kRRect:
            static_assert(0 == SkRRect::kSizeInMemory % sizeof(uint32_t));
            count += SkRRect::kSizeInMemory / sizeof(uint32_t);
            break;
        case Type::kArc:
            static_assert(0 == sizeof(SkArc) % sizeof(uint32_t));
            count += sizeof(SkArc) / sizeof(uint32_t);
            break;
        case Type::kPath: {
            // An empty path is the same as an empty shape -- only needs the state flags
            if (!this->path().isEmpty()) {
                int dataKeySize = path_key_from_data_size(this->path());
                if (dataKeySize >= 0) {
                    count += dataKeySize;
                } else {
                    count++; // Just adds the gen ID.
                }
            }
            break;
        }
        default:
            // else it's empty, which just needs the state flags for its key
            SkASSERT(this->isEmpty());
    }
    return count;
}

void Shape::writeKey(uint32_t* key, bool includeInverted) const {
    SkASSERT(this->keySize());
    SkDEBUGCODE(uint32_t* origKey = key;)

    // Every key starts with the state from the Shape (this includes path fill type,
    // and any tracked inversion, as well as the class of geometry).
    *key++ = this->stateKey(includeInverted);

    switch(this->type()) {
        case Type::kPath: {
            // An empty path is the same as an empty shape -- only needs the state flags
            if (!this->path().isEmpty()) {
                // The path's inversion must match our state in order for the path's key to suffice.
                SkASSERT(this->inverted() == this->path().isInverseFillType());

                int dataKeySize = path_key_from_data_size(this->path());
                if (dataKeySize >= 0) {
                    // We check the rest of the size in write_path_key_from_data
                    SkASSERT(key - origKey == 1);
                    write_path_key_from_data(this->path(), key);
                    return;
                } else {
                    *key++ = this->path().getGenerationID();
                }
            }
            break;
        }
        case Type::kRect:
            memcpy(key, &this->rect(), sizeof(Rect));
            key += sizeof(Rect) / sizeof(uint32_t);
            break;
        case Type::kRRect:
            this->rrect().writeToMemory(key);
            key += SkRRect::kSizeInMemory / sizeof(uint32_t);
            break;
        case Type::kArc: {
            // Write dense floats first
            memcpy(key, &fArc, sizeof(SkRect) + 2 * sizeof(float));
            key += (sizeof(SkArc) / sizeof(uint32_t) - 1);
            // Then write the final bool as an int, to make sure upper bits are set
            *key++ = fArc.isWedge() ? 1 : 0;
            break;
        }
        case Type::kLine: {
            skvx::float4 line = this->line();
            memcpy(key, &line, sizeof(skvx::float4));
            key += sizeof(skvx::float4) / sizeof(uint32_t);
            break;
        }
        default:
            // Nothing other than the flag state is needed in the key for an empty shape
            SkASSERT(this->isEmpty());
    }
    SkASSERT(key - origKey == this->keySize());
}

namespace {
SkPathFillType noninverted_fill_type(SkPathFillType fillType) {
    switch (fillType) {
        case SkPathFillType::kWinding:
        case SkPathFillType::kInverseWinding:
            return SkPathFillType::kWinding;
        case SkPathFillType::kEvenOdd:
        case SkPathFillType::kInverseEvenOdd:
            return SkPathFillType::kEvenOdd;
    }
    SkUNREACHABLE;
}
} // anonymous namespace

uint32_t Shape::stateKey(bool includeInverted) const {
    uint32_t key;
    if (includeInverted) {
        // Use the path's full fill type instead of just whether or not it's inverted.
        key = this->isPath() ? static_cast<uint32_t>(fPath.getFillType())
                             : (fInverted ? 1 : 0);
    } else {
        // Use the path's noninverted fill type.
        key = this->isPath() ? static_cast<uint32_t>(noninverted_fill_type(fPath.getFillType()))
                             : 0;
    }
    key |= ((uint32_t) fType) << 2; // fill type was 2 bits
    return key;
}

} // namespace skgpu::graphite
