/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkColorSpace.h"

static inline bool SkFloatIsFinite(float x) { return 0 == x * 0; }

//
// SkFloat3x3
//
// In memory order, values are a, b, c, d, e, f, g, h, i
//
// When applied to a color component vector (e.g. [ r, r, r ] or [ g, g, g ] we do
//
// [ r r r ] * [ a b c ] + [ g g g ] * [ d e f ] + [ b b b ] * [ g h i ]
//
// Thus in our point-on-the-right notation, the matrix looks like
//
// [ a d g ]   [ r ]
// [ b e h ] * [ g ]
// [ c f i ]   [ b ]
//
static SkFloat3x3 concat(const SkFloat3x3& left, const SkFloat3x3& rite) {
    SkFloat3x3 result;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            double tmp = 0;
            for (int i = 0; i < 3; ++i) {
                tmp += (double)left.fMat[row + i * 3] * rite.fMat[i + col * 3];
            }
            result.fMat[row + col * 3] = (double)tmp;
        }
    }
    return result;
}

static double det(const SkFloat3x3& m) {
    return (double)m.fMat[0] * m.fMat[4] * m.fMat[8] +
           (double)m.fMat[3] * m.fMat[7] * m.fMat[2] +
           (double)m.fMat[6] * m.fMat[1] * m.fMat[5] -
           (double)m.fMat[0] * m.fMat[7] * m.fMat[5] -
           (double)m.fMat[3] * m.fMat[1] * m.fMat[8] -
           (double)m.fMat[6] * m.fMat[4] * m.fMat[2];
}

static double det2x2(const SkFloat3x3& m, int a, int b, int c, int d) {
    return (double)m.fMat[a] * m.fMat[b] - (double)m.fMat[c] * m.fMat[d];
}

static SkFloat3x3 invert(const SkFloat3x3& m) {
    double d = det(m);
    SkASSERT(SkFloatIsFinite((float)d));
    double scale = 1 / d;
    SkASSERT(SkFloatIsFinite((float)scale));

    return {{
        (float)(scale * det2x2(m, 4, 8, 5, 7)),
        (float)(scale * det2x2(m, 7, 2, 8, 1)),
        (float)(scale * det2x2(m, 1, 5, 2, 4)),

        (float)(scale * det2x2(m, 6, 5, 8, 3)),
        (float)(scale * det2x2(m, 0, 8, 2, 6)),
        (float)(scale * det2x2(m, 3, 2, 5, 0)),

        (float)(scale * det2x2(m, 3, 7, 4, 6)),
        (float)(scale * det2x2(m, 6, 1, 7, 0)),
        (float)(scale * det2x2(m, 0, 4, 1, 3)),
    }};
}

void SkFloat3::dump() const {
    SkDebugf("[%7.4f %7.4f %7.4f]\n", fVec[0], fVec[1], fVec[2]);
}

void SkFloat3x3::dump() const {
    SkDebugf("[%7.4f %7.4f %7.4f] [%7.4f %7.4f %7.4f] [%7.4f %7.4f %7.4f]\n",
             fMat[0], fMat[1], fMat[2],
             fMat[3], fMat[4], fMat[5],
             fMat[6], fMat[7], fMat[8]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t gUniqueColorSpaceID;

SkColorSpace::SkColorSpace(const SkFloat3x3& toXYZD50, const SkFloat3& gamma, Named named)
    : fToXYZD50(toXYZD50)
    , fGamma(gamma)
    , fUniqueID(sk_atomic_inc(&gUniqueColorSpaceID))
    , fNamed(named)
{
    for (int i = 0; i < 3; ++i) {
        SkASSERT(SkFloatIsFinite(gamma.fVec[i]));
        for (int j = 0; j < 3; ++j) {
            SkASSERT(SkFloatIsFinite(toXYZD50.fMat[3*i + j]));
        }
    }
}

sk_sp<SkColorSpace> SkColorSpace::NewRGB(const SkFloat3x3& toXYZD50, const SkFloat3& gamma) {
    for (int i = 0; i < 3; ++i) {
        if (!SkFloatIsFinite(gamma.fVec[i]) || gamma.fVec[i] < 0) {
            return nullptr;
        }
        for (int j = 0; j < 3; ++j) {
            if (!SkFloatIsFinite(toXYZD50.fMat[3*i + j])) {
                return nullptr;
            }
        }
    }

    // check the matrix for invertibility
    float d = det(toXYZD50);
    if (!SkFloatIsFinite(d) || !SkFloatIsFinite(1 / d)) {
        return nullptr;
    }

    return sk_sp<SkColorSpace>(new SkColorSpace(toXYZD50, gamma, kUnknown_Named));
}

void SkColorSpace::dump() const {
    fToXYZD50.dump();
    fGamma.dump();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

const SkFloat3   gDevice_gamma {{ 0, 0, 0 }};
const SkFloat3x3 gDevice_toXYZD50 {{
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
}};

const SkFloat3 gSRGB_gamma {{ 2.2f, 2.2f, 2.2f }};
const SkFloat3x3 gSRGB_toXYZD50 {{
    0.4358f, 0.2224f, 0.0139f,    // * R
    0.3853f, 0.7170f, 0.0971f,    // * G
    0.1430f, 0.0606f, 0.7139f,    // * B
}};

sk_sp<SkColorSpace> SkColorSpace::NewNamed(Named named) {
    switch (named) {
        case kDevice_Named:
            return sk_sp<SkColorSpace>(new SkColorSpace(gDevice_toXYZD50, gDevice_gamma,
                                                        kDevice_Named));
        case kSRGB_Named:
            return sk_sp<SkColorSpace>(new SkColorSpace(gSRGB_toXYZD50, gSRGB_gamma, kSRGB_Named));
        default:
            break;
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkFixed.h"
#include "SkTemplates.h"

#define SkColorSpacePrintf(...)

#define return_if_false(pred, msg)                                   \
    do {                                                             \
        if (!(pred)) {                                               \
            SkColorSpacePrintf("Invalid ICC Profile: %s.\n", (msg)); \
            return false;                                            \
        }                                                            \
    } while (0)

#define return_null(msg)                                             \
    do {                                                             \
        SkColorSpacePrintf("Invalid ICC Profile: %s.\n", (msg));     \
        return nullptr;                                              \
    } while (0)

static uint16_t read_big_endian_short(const uint8_t* ptr) {
    return ptr[0] << 8 | ptr[1];
}

static uint32_t read_big_endian_int(const uint8_t* ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

// This is equal to the header size according to the ICC specification (128)
// plus the size of the tag count (4).  We include the tag count since we
// always require it to be present anyway.
static const size_t kICCHeaderSize = 132;

// Contains a signature (4), offset (4), and size (4).
static const size_t kICCTagTableEntrySize = 12;

static const uint32_t kRGB_ColorSpace  = SkSetFourByteTag('R', 'G', 'B', ' ');
static const uint32_t kGray_ColorSpace = SkSetFourByteTag('G', 'R', 'A', 'Y');

struct ICCProfileHeader {
    // TODO (msarett):
    // Can we ignore less of these fields?
    uint32_t fSize;
    uint32_t fCMMType_ignored;
    uint32_t fVersion;
    uint32_t fClassProfile;
    uint32_t fColorSpace;
    uint32_t fPCS;
    uint32_t fDateTime_ignored[3];
    uint32_t fSignature;
    uint32_t fPlatformTarget_ignored;
    uint32_t fFlags_ignored;
    uint32_t fManufacturer_ignored;
    uint32_t fDeviceModel_ignored;
    uint32_t fDeviceAttributes_ignored[2];
    uint32_t fRenderingIntent;
    uint32_t fIlluminantXYZ_ignored[3];
    uint32_t fCreator_ignored;
    uint32_t fProfileId_ignored[4];
    uint32_t fReserved_ignored[7];
    uint32_t fTagCount;

    void init(const uint8_t* src, size_t len) {
        SkASSERT(kICCHeaderSize == sizeof(*this));

        uint32_t* dst = (uint32_t*) this;
        for (uint32_t i = 0; i < kICCHeaderSize / 4; i++, src+=4) {
            dst[i] = read_big_endian_int(src);
        }
    }

    bool valid() const {
        // TODO (msarett):
        // For now it's nice to fail loudly on invalid inputs.  But, can we
        // recover from some of these errors?

        return_if_false(fSize >= kICCHeaderSize, "Size is too small");

        uint8_t majorVersion = fVersion >> 24;
        return_if_false(majorVersion <= 4, "Unsupported version");

        const uint32_t kDisplay_Profile = SkSetFourByteTag('m', 'n', 't', 'r');
        const uint32_t kInput_Profile   = SkSetFourByteTag('s', 'c', 'n', 'r');
        const uint32_t kOutput_Profile  = SkSetFourByteTag('p', 'r', 't', 'r');
        // TODO (msarett):
        // Should we also support DeviceLink, ColorSpace, Abstract, or NamedColor?
        return_if_false(fClassProfile == kDisplay_Profile ||
                        fClassProfile == kInput_Profile ||
                        fClassProfile == kOutput_Profile,
                        "Unsupported class profile");

        // TODO (msarett):
        // There are many more color spaces that we could try to support.
        return_if_false(fColorSpace == kRGB_ColorSpace || fColorSpace == kGray_ColorSpace,
                        "Unsupported color space");

        const uint32_t kXYZ_PCSSpace = SkSetFourByteTag('X', 'Y', 'Z', ' ');
        // TODO (msarett):
        // Can we support PCS LAB as well?
        return_if_false(fPCS == kXYZ_PCSSpace, "Unsupported PCS space");

        return_if_false(fSignature == SkSetFourByteTag('a', 'c', 's', 'p'), "Bad signature");

        // TODO (msarett):
        // Should we treat different rendering intents differently?
        // Valid rendering intents include kPerceptual (0), kRelative (1),
        // kSaturation (2), and kAbsolute (3).
        return_if_false(fRenderingIntent <= 3, "Bad rendering intent");

        return_if_false(fTagCount <= 100, "Too many tags");

        return true;
    }
};

struct ICCTag {
    uint32_t fSignature;
    uint32_t fOffset;
    uint32_t fLength;

    const uint8_t* init(const uint8_t* src) {
        fSignature = read_big_endian_int(src);
        fOffset = read_big_endian_int(src + 4);
        fLength = read_big_endian_int(src + 8);
        return src + 12;
    }

    bool valid(size_t len) {
        return_if_false(fOffset + fLength <= len, "Tag too large for ICC profile");
        return true;
    }

    const uint8_t* addr(const uint8_t* src) const {
        return src + fOffset;
    }

    static const ICCTag* Find(const ICCTag tags[], int count, uint32_t signature) {
        for (int i = 0; i < count; ++i) {
            if (tags[i].fSignature == signature) {
                return &tags[i];
            }
        }
        return nullptr;
    }
};

// TODO (msarett):
// Should we recognize more tags?
static const uint32_t kTAG_rXYZ = SkSetFourByteTag('r', 'X', 'Y', 'Z');
static const uint32_t kTAG_gXYZ = SkSetFourByteTag('g', 'X', 'Y', 'Z');
static const uint32_t kTAG_bXYZ = SkSetFourByteTag('b', 'X', 'Y', 'Z');
static const uint32_t kTAG_rTRC = SkSetFourByteTag('r', 'T', 'R', 'C');
static const uint32_t kTAG_gTRC = SkSetFourByteTag('g', 'T', 'R', 'C');
static const uint32_t kTAG_bTRC = SkSetFourByteTag('b', 'T', 'R', 'C');

bool load_xyz(float dst[3], const uint8_t* src, size_t len) {
    if (len < 20) {
        SkColorSpacePrintf("XYZ tag is too small (%d bytes)", len);
        return false;
    }

    dst[0] = SkFixedToFloat(read_big_endian_int(src + 8));
    dst[1] = SkFixedToFloat(read_big_endian_int(src + 12));
    dst[2] = SkFixedToFloat(read_big_endian_int(src + 16));
    SkColorSpacePrintf("XYZ %g %g %g\n", dst[0], dst[1], dst[2]);
    return true;
}

static const uint32_t kTAG_CurveType     = SkSetFourByteTag('c', 'u', 'r', 'v');
static const uint32_t kTAG_ParaCurveType = SkSetFourByteTag('p', 'a', 'r', 'a');

static bool load_gamma(float* gamma, const uint8_t* src, size_t len) {
    if (len < 14) {
        SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
        return false;
    }

    uint32_t type = read_big_endian_int(src);
    switch (type) {
        case kTAG_CurveType: {
            uint32_t count = read_big_endian_int(src + 8);
            if (0 == count) {
                return false;
            }

            const uint16_t* table = (const uint16_t*) (src + 12);
            if (1 == count) {
                // Table entry is the exponent (bias 256).
                uint16_t value = read_big_endian_short((const uint8_t*) table);
                *gamma = value / 256.0f;
                SkColorSpacePrintf("gamma %d %g\n", value, *gamma);
                return true;
            }

            // Check length again if we have a table.
            if (len < 12 + 2 * count) {
                SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                return false;
            }

            // Print the interpolation table.  For now, we ignore this and guess 2.2f.
            for (uint32_t i = 0; i < count; i++) {
                SkColorSpacePrintf("curve[%d] %d\n", i,
                        read_big_endian_short((const uint8_t*) &table[i]));
            }

            *gamma = 2.2f;
            return true;
        }
        case kTAG_ParaCurveType:
            // Guess 2.2f.
            SkColorSpacePrintf("parametric curve\n");
            *gamma = 2.2f;
            return true;
        default:
            SkColorSpacePrintf("Unsupported gamma tag type %d\n", type);
            return false;
    }
}

sk_sp<SkColorSpace> SkColorSpace::NewICC(const void* base, size_t len) {
    const uint8_t* ptr = (const uint8_t*) base;

    if (len < kICCHeaderSize) {
        return_null("Data is not large enough to contain an ICC profile");
    }

    // Read the ICC profile header and check to make sure that it is valid.
    ICCProfileHeader header;
    header.init(ptr, len);
    if (!header.valid()) {
        return nullptr;
    }

    // Adjust ptr and len before reading the tags.
    if (len < header.fSize) {
        SkColorSpacePrintf("ICC profile might be truncated.\n");
    } else if (len > header.fSize) {
        SkColorSpacePrintf("Caller provided extra data beyond the end of the ICC profile.\n");
        len = header.fSize;
    }
    ptr += kICCHeaderSize;
    len -= kICCHeaderSize;

    // Parse tag headers.
    uint32_t tagCount = header.fTagCount;
    SkColorSpacePrintf("ICC profile contains %d tags.\n", tagCount);
    if (len < kICCTagTableEntrySize * tagCount) {
        return_null("Not enough input data to read tag table entries");
    }

    SkAutoTArray<ICCTag> tags(tagCount);
    for (uint32_t i = 0; i < tagCount; i++) {
        ptr = tags[i].init(ptr);
        SkColorSpacePrintf("[%d] %c%c%c%c %d %d\n", i, (tags[i].fSignature >> 24) & 0xFF,
                (tags[i].fSignature >> 16) & 0xFF, (tags[i].fSignature >>  8) & 0xFF,
                (tags[i].fSignature >>  0) & 0xFF, tags[i].fOffset, tags[i].fLength);

        if (!tags[i].valid(kICCHeaderSize + len)) {
            return_null("Tag is too large to fit in ICC profile");
        }
    }

    // Load our XYZ and gamma matrices.
    SkFloat3x3 toXYZ;
    SkFloat3 gamma {{ 1.0f, 1.0f, 1.0f }};
    switch (header.fColorSpace) {
        case kRGB_ColorSpace: {
            const ICCTag* r = ICCTag::Find(tags.get(), tagCount, kTAG_rXYZ);
            const ICCTag* g = ICCTag::Find(tags.get(), tagCount, kTAG_gXYZ);
            const ICCTag* b = ICCTag::Find(tags.get(), tagCount, kTAG_bXYZ);
            if (!r || !g || !b) {
                return_null("Need rgb tags for XYZ space");
            }

            if (!load_xyz(&toXYZ.fMat[0], r->addr((const uint8_t*) base), r->fLength) ||
                !load_xyz(&toXYZ.fMat[3], g->addr((const uint8_t*) base), g->fLength) ||
                !load_xyz(&toXYZ.fMat[6], b->addr((const uint8_t*) base), b->fLength))
            {
                return_null("Need valid rgb tags for XYZ space");
            }

            r = ICCTag::Find(tags.get(), tagCount, kTAG_rTRC);
            g = ICCTag::Find(tags.get(), tagCount, kTAG_gTRC);
            b = ICCTag::Find(tags.get(), tagCount, kTAG_bTRC);
            if (!r || !load_gamma(&gamma.fVec[0], r->addr((const uint8_t*) base), r->fLength)) {
                SkColorSpacePrintf("Failed to read R gamma tag.\n");
            }
            if (!g || !load_gamma(&gamma.fVec[1], g->addr((const uint8_t*) base), g->fLength)) {
                SkColorSpacePrintf("Failed to read G gamma tag.\n");
            }
            if (!b || !load_gamma(&gamma.fVec[2], b->addr((const uint8_t*) base), b->fLength)) {
                SkColorSpacePrintf("Failed to read B gamma tag.\n");
            }
            return SkColorSpace::NewRGB(toXYZ, gamma);
        }
        default:
            break;
    }

    return_null("ICC profile contains unsupported colorspace");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkColorSpace::Result SkColorSpace::Concat(const SkColorSpace* src, const SkColorSpace* dst,
                                          SkFloat3x3* result) {
    if (!src || !dst || (src->named() == kDevice_Named) || (src->named() == dst->named())) {
        if (result) {
            *result = {{ 1, 0, 0, 0, 1, 0, 0, 0, 1 }};
        }
        return kIdentity_Result;
    }
    if (result) {
        *result = concat(src->fToXYZD50, invert(dst->fToXYZD50));
    }
    return kNormal_Result;
}

#include "SkColor.h"
#include "SkNx.h"
#include "SkPM4f.h"

void SkApply3x3ToPM4f(const SkFloat3x3& m, const SkPM4f src[], SkPM4f dst[], int count) {
    SkASSERT(1 == SkPM4f::G);
    SkASSERT(3 == SkPM4f::A);

    Sk4f cr, cg, cb;
    cg = Sk4f::Load(m.fMat + 3);
    if (0 == SkPM4f::R) {
        SkASSERT(2 == SkPM4f::B);
        cr = Sk4f::Load(m.fMat + 0);
        cb = Sk4f(m.fMat[6], m.fMat[7], m.fMat[8], 0);
    } else {
        SkASSERT(0 == SkPM4f::B);
        SkASSERT(2 == SkPM4f::R);
        cb = Sk4f::Load(m.fMat + 0);
        cr = Sk4f(m.fMat[6], m.fMat[7], m.fMat[8], 0);
    }
    cr = cr * Sk4f(1, 1, 1, 0);
    cg = cg * Sk4f(1, 1, 1, 0);
    cb = cb * Sk4f(1, 1, 1, 0);

    for (int i = 0; i < count; ++i) {
        Sk4f r = Sk4f(src[i].fVec[SkPM4f::R]);
        Sk4f g = Sk4f(src[i].fVec[SkPM4f::G]);
        Sk4f b = Sk4f(src[i].fVec[SkPM4f::B]);
        Sk4f a = Sk4f(0, 0, 0, src[i].fVec[SkPM4f::A]);
        (cr * r + cg * g + cb * b + a).store(&dst[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkColorSpace::Test() {
    SkFloat3x3 mat {{ 2, 0, 0, 0, 3, 0, 0, 0, 4 }};
    SkFloat3x3 inv = invert(mat);
    mat.dump();
    inv.dump();
    concat(mat, inv).dump();
    concat(inv, mat).dump();
    SkDebugf("\n");

    mat = gSRGB_toXYZD50;
    inv = invert(mat);
    mat.dump();
    inv.dump();
    concat(mat, inv).dump();
    concat(inv, mat).dump();
    SkDebugf("\n");

    sk_sp<SkColorSpace> cs0(SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named));
    sk_sp<SkColorSpace> cs1(SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named));

    cs0->dump();
    cs1->dump();
    SkFloat3x3 xform;
    (void)SkColorSpace::Concat(cs0.get(), cs1.get(), &xform);
    xform.dump();
    SkDebugf("\n");
}

// D65 white point of Rec.  709 [8] are:
//
// D65 white-point in unit luminance XYZ = 0.9505, 1.0000, 1.0890
//
//          R           G           B           white
//   x      0.640       0.300       0.150       0.3127
//   y      0.330       0.600       0.060       0.3290
//   z      0.030       0.100       0.790       0.3582
