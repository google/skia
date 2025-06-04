/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkBuffer.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"

#include <cstddef>
#include <cstdint>
#include <optional>

enum SerializationOffsets {
    kType_SerializationShift = 28,       // requires 4 bits
    kDirection_SerializationShift = 26,  // requires 2 bits
    kFillType_SerializationShift = 8,    // requires 8 bits
    // low-8-bits are version
    kVersion_SerializationMask = 0xFF,
};

enum SerializationVersions {
    // kPathPrivFirstDirection_Version = 1,
    // kPathPrivLastMoveToIndex_Version = 2,
    // kPathPrivTypeEnumVersion = 3,
    kJustPublicData_Version = 4,            // introduced Feb/2018
    kVerbsAreStoredForward_Version = 5,     // introduced Sept/2019

    kMin_Version     = kJustPublicData_Version,
    kCurrent_Version = kVerbsAreStoredForward_Version
};

enum SerializationType {
    kGeneral = 0,
    kRRect = 1
};

static unsigned extract_version(uint32_t packed) {
    return packed & kVersion_SerializationMask;
}

static SkPathFillType extract_filltype(uint32_t packed) {
    return static_cast<SkPathFillType>((packed >> kFillType_SerializationShift) & 0x3);
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

    int firstDir = isCCW ? (int)SkPathFirstDirection::kCCW : (int)SkPathFirstDirection::kCW;
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
    buffer.write(fPathRef->verbsBegin(), vbs * sizeof(uint8_t));
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

size_t SkPath::readAsRRect(const void* storage, size_t length) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        return 0;
    }

    SkASSERT(extract_serializationtype(packed) == SerializationType::kRRect);

    uint8_t dir = (packed >> kDirection_SerializationShift) & 0x3;
    SkPathFillType fillType = extract_filltype(packed);

    SkPathDirection rrectDir;
    SkRRect rrect;
    int32_t start;
    switch (dir) {
        case (int)SkPathFirstDirection::kCW:
            rrectDir = SkPathDirection::kCW;
            break;
        case (int)SkPathFirstDirection::kCCW:
            rrectDir = SkPathDirection::kCCW;
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

size_t SkPath::readFromMemory(const void* storage, size_t length) {
    size_t bytesRead;
    std::optional<SkPath> path = SkPath::ReadFromMemory(storage, length, &bytesRead);
    if (path) {
        *this = path.value();
    }
    return bytesRead;
}

#define RETURN_PATH_AND_BYTES(p, b) \
    do { if (bytesRead) { *bytesRead = b; }; return p; } while (0)

std::optional<SkPath> SkPath::ReadFromMemory(const void* storage, size_t length, size_t* bytesRead) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        RETURN_PATH_AND_BYTES(std::nullopt, 0);
    }
    unsigned version = extract_version(packed);

    const bool verbsAreForward = (version == kVerbsAreStoredForward_Version);
    if (!verbsAreForward && version != kJustPublicData_Version) SK_UNLIKELY {
        // Old/unsupported version.
        RETURN_PATH_AND_BYTES(std::nullopt, 0);
    }

    SkPath path;
    size_t tmp;
    switch (extract_serializationtype(packed)) {
        case SerializationType::kRRect:
            tmp = path.readAsRRect(storage, length);
            RETURN_PATH_AND_BYTES(path, tmp);
        case SerializationType::kGeneral:
            break;  // fall out
        default:
            RETURN_PATH_AND_BYTES(std::nullopt, 0);
    }

    // To minimize the number of reads done a structure with the counts is used.
    struct {
      uint32_t pts, cnx, vbs;
    } counts;
    if (!buffer.read(&counts, sizeof(counts))) {
        RETURN_PATH_AND_BYTES(std::nullopt, 0);
    }

    const SkPoint* points = buffer.skipCount<SkPoint>(counts.pts);
    const SkScalar* conics = buffer.skipCount<SkScalar>(counts.cnx);
    const uint8_t* verbs = buffer.skipCount<uint8_t>(counts.vbs);
    buffer.skipToAlign4();
    if (!buffer.isValid()) {
        RETURN_PATH_AND_BYTES(std::nullopt, 0);
    }
    SkASSERT(buffer.pos() <= length);

    if (counts.vbs == 0) {
        if (counts.pts == 0 && counts.cnx == 0) {
            path.setFillType(extract_filltype(packed));
            if (bytesRead) {
                *bytesRead = buffer.pos();
            }
            return path;
        }
        // No verbs but points and/or conic weights is a not a valid path.
        RETURN_PATH_AND_BYTES(std::nullopt, 0);
    }

    SkAutoMalloc reversedStorage;
    if (!verbsAreForward) SK_UNLIKELY {
      uint8_t* tmpVerbs = (uint8_t*)reversedStorage.reset(counts.vbs);
        for (unsigned i = 0; i < counts.vbs; ++i) {
            tmpVerbs[i] = verbs[counts.vbs - i - 1];
        }
        verbs = tmpVerbs;
    }

    SkPathVerbAnalysis analysis = SkPathPriv::AnalyzeVerbs({verbs, counts.vbs});
    if (!analysis.valid || analysis.points != counts.pts || analysis.weights != counts.cnx) {
        RETURN_PATH_AND_BYTES(std::nullopt, 0);
    }
    path = SkPathPriv::MakePath(analysis, points, verbs, counts.vbs, conics,
                                extract_filltype(packed), false);

    RETURN_PATH_AND_BYTES(path,buffer.pos());
}

#undef RETURN_PATH_AND_BYTES
