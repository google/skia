/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkMath.h"
#include "include/private/SkPathRef.h"
#include "include/private/SkTo.h"
#include "src/core/SkBuffer.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkSafeMath.h"

#include <cmath>

enum SerializationOffsets {
    kType_SerializationShift = 28,       // requires 4 bits
    kDirection_SerializationShift = 26,  // requires 2 bits
    kFillType_SerializationShift = 8,    // requires 8 bits
    // low-8-bits are version
    kVersion_SerializationMask = 0xFF,
};

enum SerializationVersions {
    // kPathPrivFirstDirection_Version = 1,
    kPathPrivLastMoveToIndex_Version = 2,
    kPathPrivTypeEnumVersion = 3,
    kJustPublicData_Version = 4,    // introduced Feb/2018

    kCurrent_Version = kJustPublicData_Version
};

enum SerializationType {
    kGeneral = 0,
    kRRect = 1
};

static unsigned extract_version(uint32_t packed) {
    return packed & kVersion_SerializationMask;
}

static SkPath::FillType extract_filltype(uint32_t packed) {
    return static_cast<SkPath::FillType>((packed >> kFillType_SerializationShift) & 0x3);
}

static SerializationType extract_serializationtype(uint32_t packed) {
    return static_cast<SerializationType>((packed >> kType_SerializationShift) & 0xF);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t SkPath::writeToMemoryAsRRect(void* storage) const {
    SkRect oval;
    SkRRect rrect;
    bool isCCW;
    unsigned start;
    if (fPathRef->isOval(&oval, &isCCW, &start)) {
        rrect.setOval(oval);
        // Convert to rrect start indices.
        start *= 2;
    } else if (!fPathRef->isRRect(&rrect, &isCCW, &start)) {
        return 0;
    }

    // packed header, rrect, start index.
    const size_t sizeNeeded = sizeof(int32_t) + SkRRect::kSizeInMemory + sizeof(int32_t);
    if (!storage) {
        return sizeNeeded;
    }

    int firstDir = isCCW ? SkPathPriv::kCCW_FirstDirection : SkPathPriv::kCW_FirstDirection;
    int32_t packed = (fFillType << kFillType_SerializationShift) |
                     (firstDir << kDirection_SerializationShift) |
                     (SerializationType::kRRect << kType_SerializationShift) |
                     kCurrent_Version;

    SkWBuffer buffer(storage);
    buffer.write32(packed);
    SkRRectPriv::WriteToBuffer(rrect, &buffer);
    buffer.write32(SkToS32(start));
    buffer.padToAlign4();
    SkASSERT(sizeNeeded == buffer.pos());
    return buffer.pos();
}

size_t SkPath::writeToMemory(void* storage) const {
    SkDEBUGCODE(this->validate();)

    if (size_t bytes = this->writeToMemoryAsRRect(storage)) {
        return bytes;
    }

    int32_t packed = (fFillType << kFillType_SerializationShift) |
                     (SerializationType::kGeneral << kType_SerializationShift) |
                     kCurrent_Version;

    int32_t pts = fPathRef->countPoints();
    int32_t cnx = fPathRef->countWeights();
    int32_t vbs = fPathRef->countVerbs();

    SkSafeMath safe;
    size_t size = 4 * sizeof(int32_t);
    size = safe.add(size, safe.mul(pts, sizeof(SkPoint)));
    size = safe.add(size, safe.mul(cnx, sizeof(SkScalar)));
    size = safe.add(size, safe.mul(vbs, sizeof(uint8_t)));
    size = safe.alignUp(size, 4);
    if (!safe) {
        return 0;
    }
    if (!storage) {
        return size;
    }

    SkWBuffer buffer(storage);
    buffer.write32(packed);
    buffer.write32(pts);
    buffer.write32(cnx);
    buffer.write32(vbs);
    buffer.write(fPathRef->points(), pts * sizeof(SkPoint));
    buffer.write(fPathRef->conicWeights(), cnx * sizeof(SkScalar));
    buffer.write(fPathRef->verbsMemBegin(), vbs * sizeof(uint8_t));
    buffer.padToAlign4();

    SkASSERT(buffer.pos() == size);
    return size;
}

sk_sp<SkData> SkPath::serialize() const {
    size_t size = this->writeToMemory(nullptr);
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    this->writeToMemory(data->writable_data());
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// reading

size_t SkPath::readFromMemory(const void* storage, size_t length) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        return 0;
    }
    unsigned version = extract_version(packed);
    if (version <= kPathPrivTypeEnumVersion) {
        return this->readFromMemory_LE3(storage, length);
    }
    if (version == kJustPublicData_Version) {
        return this->readFromMemory_EQ4(storage, length);
    }
    return 0;
}

size_t SkPath::readAsRRect(const void* storage, size_t length) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        return 0;
    }

    SkASSERT(extract_serializationtype(packed) == SerializationType::kRRect);

    uint8_t dir = (packed >> kDirection_SerializationShift) & 0x3;
    FillType fillType = extract_filltype(packed);

    Direction rrectDir;
    SkRRect rrect;
    int32_t start;
    switch (dir) {
        case SkPathPriv::kCW_FirstDirection:
            rrectDir = kCW_Direction;
            break;
        case SkPathPriv::kCCW_FirstDirection:
            rrectDir = kCCW_Direction;
            break;
        default:
            return 0;
    }
    if (!SkRRectPriv::ReadFromBuffer(&buffer, &rrect)) {
        return 0;
    }
    if (!buffer.readS32(&start) || start != SkTPin(start, 0, 7)) {
        return 0;
    }
    this->reset();
    this->addRRect(rrect, rrectDir, SkToUInt(start));
    this->setFillType(fillType);
    buffer.skipToAlign4();
    return buffer.pos();
}

size_t SkPath::readFromMemory_EQ4(const void* storage, size_t length) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        return 0;
    }

    SkASSERT(extract_version(packed) == 4);

    switch (extract_serializationtype(packed)) {
        case SerializationType::kRRect:
            return this->readAsRRect(storage, length);
        case SerializationType::kGeneral:
            break;  // fall through
        default:
            return 0;
    }

    int32_t pts, cnx, vbs;
    if (!buffer.readS32(&pts) || !buffer.readS32(&cnx) || !buffer.readS32(&vbs)) {
        return 0;
    }

    const SkPoint* points = buffer.skipCount<SkPoint>(pts);
    const SkScalar* conics = buffer.skipCount<SkScalar>(cnx);
    const uint8_t* verbs = buffer.skipCount<uint8_t>(vbs);
    buffer.skipToAlign4();
    if (!buffer.isValid()) {
        return 0;
    }
    SkASSERT(buffer.pos() <= length);

#define CHECK_POINTS_CONICS(p, c)       \
    do {                                \
        if (p && ((pts -= p) < 0)) {    \
            return 0;                   \
        }                               \
        if (c && ((cnx -= c) < 0)) {    \
            return 0;                   \
        }                               \
    } while (0)

    SkPath tmp;
    tmp.setFillType(extract_filltype(packed));
    tmp.incReserve(pts);
    for (int i = vbs - 1; i >= 0; --i) {
        switch (verbs[i]) {
            case kMove_Verb:
                CHECK_POINTS_CONICS(1, 0);
                tmp.moveTo(*points++);
                break;
            case kLine_Verb:
                CHECK_POINTS_CONICS(1, 0);
                tmp.lineTo(*points++);
                break;
            case kQuad_Verb:
                CHECK_POINTS_CONICS(2, 0);
                tmp.quadTo(points[0], points[1]);
                points += 2;
                break;
            case kConic_Verb:
                CHECK_POINTS_CONICS(2, 1);
                tmp.conicTo(points[0], points[1], *conics++);
                points += 2;
                break;
            case kCubic_Verb:
                CHECK_POINTS_CONICS(3, 0);
                tmp.cubicTo(points[0], points[1], points[2]);
                points += 3;
                break;
            case kClose_Verb:
                tmp.close();
                break;
            default:
                return 0;   // bad verb
        }
    }
#undef CHECK_POINTS_CONICS
    if (pts || cnx) {
        return 0;   // leftover points and/or conics
    }

    *this = std::move(tmp);
    return buffer.pos();
}

size_t SkPath::readFromMemory_LE3(const void* storage, size_t length) {
    SkRBuffer buffer(storage, length);

    int32_t packed;
    if (!buffer.readS32(&packed)) {
        return 0;
    }

    unsigned version = extract_version(packed);
    SkASSERT(version <= 3);

    FillType fillType = extract_filltype(packed);
    if (version >= kPathPrivTypeEnumVersion) {
        switch (extract_serializationtype(packed)) {
            case SerializationType::kRRect:
                return this->readAsRRect(storage, length);
            case SerializationType::kGeneral:
                // Fall through to general path deserialization
                break;
            default:
                return 0;
        }
    }
    if (version >= kPathPrivLastMoveToIndex_Version && !buffer.readS32(&fLastMoveToIndex)) {
        return 0;
    }

    // These are written into the serialized data but we no longer use them in the deserialized
    // path. If convexity is corrupted it may cause the GPU backend to make incorrect
    // rendering choices, possibly crashing. We set them to unknown so that they'll be recomputed if
    // requested.
    fConvexity = kUnknown_Convexity;
    fFirstDirection = SkPathPriv::kUnknown_FirstDirection;

    fFillType = fillType;
    fIsVolatile = 0;
    SkPathRef* pathRef = SkPathRef::CreateFromBuffer(&buffer);
    if (!pathRef) {
        return 0;
    }

    fPathRef.reset(pathRef);
    SkDEBUGCODE(this->validate();)
    buffer.skipToAlign4();
    return buffer.pos();
}

