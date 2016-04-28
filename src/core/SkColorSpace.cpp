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

SkColorSpace::SkColorSpace(const SkFloat3& gamma, const SkFloat3x3& toXYZD50, Named named)
    : fGamma(gamma)
    , fToXYZD50(toXYZD50)
    , fToXYZOffset({{ 0.0f, 0.0f, 0.0f }})
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

SkColorSpace::SkColorSpace(SkColorLookUpTable colorLUT, const SkFloat3& gamma,
                           const SkFloat3x3& toXYZD50, const SkFloat3& toXYZOffset)
    : fColorLUT(std::move(colorLUT))
    , fGamma(gamma)
    , fToXYZD50(toXYZD50)
    , fToXYZOffset(toXYZOffset)
    , fUniqueID(sk_atomic_inc(&gUniqueColorSpaceID))
    , fNamed(kUnknown_Named)
{}

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

    return sk_sp<SkColorSpace>(new SkColorSpace(gamma, toXYZD50, kUnknown_Named));
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
            return sk_sp<SkColorSpace>(new SkColorSpace(gDevice_gamma, gDevice_toXYZD50,
                                                        kDevice_Named));
        case kSRGB_Named:
            return sk_sp<SkColorSpace>(new SkColorSpace(gSRGB_gamma, gSRGB_toXYZD50, kSRGB_Named));
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

static uint32_t read_big_endian_uint(const uint8_t* ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

static int32_t read_big_endian_int(const uint8_t* ptr) {
    return (int32_t) read_big_endian_uint(ptr);
}

static bool color_space_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.01f;
}

// This is equal to the header size according to the ICC specification (128)
// plus the size of the tag count (4).  We include the tag count since we
// always require it to be present anyway.
static const size_t kICCHeaderSize = 132;

// Contains a signature (4), offset (4), and size (4).
static const size_t kICCTagTableEntrySize = 12;

static const uint32_t kRGB_ColorSpace  = SkSetFourByteTag('R', 'G', 'B', ' ');

struct ICCProfileHeader {
    uint32_t fSize;

    // No reason to care about the preferred color management module (ex: Adobe, Apple, etc.).
    // We're always going to use this one.
    uint32_t fCMMType_ignored;

    uint32_t fVersion;
    uint32_t fProfileClass;
    uint32_t fInputColorSpace;
    uint32_t fPCS;
    uint32_t fDateTime_ignored[3];
    uint32_t fSignature;

    // Indicates the platform that this profile was created for (ex: Apple, Microsoft).  This
    // doesn't really matter to us.
    uint32_t fPlatformTarget_ignored;

    // Flags can indicate:
    // (1) Whether this profile was embedded in a file.  This flag is consistently wrong.
    //     Ex: The profile came from a file but indicates that it did not.
    // (2) Whether we are allowed to use the profile independently of the color data.  If set,
    //     this may allow us to use the embedded profile for testing separate from the original
    //     image.
    uint32_t fFlags_ignored;

    // We support many output devices.  It doesn't make sense to think about the attributes of
    // the device in the context of the image profile.
    uint32_t fDeviceManufacturer_ignored;
    uint32_t fDeviceModel_ignored;
    uint32_t fDeviceAttributes_ignored[2];

    uint32_t fRenderingIntent;
    int32_t  fIlluminantXYZ[3];

    // We don't care who created the profile.
    uint32_t fCreator_ignored;

    // This is an MD5 checksum.  Could be useful for checking if profiles are equal.
    uint32_t fProfileId_ignored[4];

    // Reserved for future use.
    uint32_t fReserved_ignored[7];

    uint32_t fTagCount;

    void init(const uint8_t* src, size_t len) {
        SkASSERT(kICCHeaderSize == sizeof(*this));

        uint32_t* dst = (uint32_t*) this;
        for (uint32_t i = 0; i < kICCHeaderSize / 4; i++, src+=4) {
            dst[i] = read_big_endian_uint(src);
        }
    }

    bool valid() const {
        return_if_false(fSize >= kICCHeaderSize, "Size is too small");

        uint8_t majorVersion = fVersion >> 24;
        return_if_false(majorVersion <= 4, "Unsupported version");

        // These are the three basic classes of profiles that we might expect to see embedded
        // in images.  Four additional classes exist, but they generally are used as a convenient
        // way for CMMs to store calculated transforms.
        const uint32_t kDisplay_Profile = SkSetFourByteTag('m', 'n', 't', 'r');
        const uint32_t kInput_Profile   = SkSetFourByteTag('s', 'c', 'n', 'r');
        const uint32_t kOutput_Profile  = SkSetFourByteTag('p', 'r', 't', 'r');
        return_if_false(fProfileClass == kDisplay_Profile ||
                        fProfileClass == kInput_Profile ||
                        fProfileClass == kOutput_Profile,
                        "Unsupported profile");

        // TODO (msarett):
        // All the profiles we've tested so far use RGB as the input color space.
        return_if_false(fInputColorSpace == kRGB_ColorSpace, "Unsupported color space");

        // TODO (msarett):
        // All the profiles we've tested so far use XYZ as the profile connection space.
        const uint32_t kXYZ_PCSSpace = SkSetFourByteTag('X', 'Y', 'Z', ' ');
        return_if_false(fPCS == kXYZ_PCSSpace, "Unsupported PCS space");

        return_if_false(fSignature == SkSetFourByteTag('a', 'c', 's', 'p'), "Bad signature");

        // TODO (msarett):
        // Should we treat different rendering intents differently?
        // Valid rendering intents include kPerceptual (0), kRelative (1),
        // kSaturation (2), and kAbsolute (3).
        return_if_false(fRenderingIntent <= 3, "Bad rendering intent");

        return_if_false(color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[0]), 0.96420f) &&
                        color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[1]), 1.00000f) &&
                        color_space_almost_equal(SkFixedToFloat(fIlluminantXYZ[2]), 0.82491f),
                        "Illuminant must be D50");

        return_if_false(fTagCount <= 100, "Too many tags");

        return true;
    }
};

struct ICCTag {
    uint32_t fSignature;
    uint32_t fOffset;
    uint32_t fLength;

    const uint8_t* init(const uint8_t* src) {
        fSignature = read_big_endian_uint(src);
        fOffset = read_big_endian_uint(src + 4);
        fLength = read_big_endian_uint(src + 8);
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

static const uint32_t kTAG_rXYZ = SkSetFourByteTag('r', 'X', 'Y', 'Z');
static const uint32_t kTAG_gXYZ = SkSetFourByteTag('g', 'X', 'Y', 'Z');
static const uint32_t kTAG_bXYZ = SkSetFourByteTag('b', 'X', 'Y', 'Z');
static const uint32_t kTAG_rTRC = SkSetFourByteTag('r', 'T', 'R', 'C');
static const uint32_t kTAG_gTRC = SkSetFourByteTag('g', 'T', 'R', 'C');
static const uint32_t kTAG_bTRC = SkSetFourByteTag('b', 'T', 'R', 'C');
static const uint32_t kTAG_A2B0 = SkSetFourByteTag('A', '2', 'B', '0');

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

// FIXME (msarett):
// We need to handle the possibility that the gamma curve does not correspond to 2.2f.
static bool load_gammas(float* gammas, uint32_t numGammas, const uint8_t* src, size_t len) {
    for (uint32_t i = 0; i < numGammas; i++) {
        if (len < 12) {
            // FIXME (msarett):
            // We could potentially return false here after correctly parsing *some* of the
            // gammas correctly.  Should we somehow try to indicate a partial success?
            SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
            return false;
        }

        // We need to count the number of bytes in the tag, so we are able to move to the
        // next tag on the next loop iteration.
        size_t tagBytes;

        uint32_t type = read_big_endian_uint(src);
        switch (type) {
            case kTAG_CurveType: {
                uint32_t count = read_big_endian_uint(src + 8);
                tagBytes = 12 + count * 2;
                if (0 == count) {
                    // Some tags require a gamma curve, but the author doesn't actually want
                    // to transform the data.  In this case, it is common to see a curve with
                    // a count of 0.
                    gammas[i] = 1.0f;
                    break;
                } else if (len < 12 + 2 * count) {
                    SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                    return false;
                }

                const uint16_t* table = (const uint16_t*) (src + 12);
                if (1 == count) {
                    // Table entry is the exponent (bias 256).
                    uint16_t value = read_big_endian_short((const uint8_t*) table);
                    gammas[i] = value / 256.0f;
                    SkColorSpacePrintf("gamma %d %g\n", value, *gamma);
                    break;
                }

                // Print the interpolation table.  For now, we ignore this and guess 2.2f.
                for (uint32_t j = 0; j < count; j++) {
                    SkColorSpacePrintf("curve[%d] %d\n", j,
                            read_big_endian_short((const uint8_t*) &table[j]));
                }

                gammas[i] = 2.2f;
                break;
            }
            case kTAG_ParaCurveType:
                // Guess 2.2f.
                SkColorSpacePrintf("parametric curve\n");
                gammas[i] = 2.2f;

                switch(read_big_endian_short(src + 8)) {
                    case 0:
                        tagBytes = 12 + 4;
                        break;
                    case 1:
                        tagBytes = 12 + 12;
                        break;
                    case 2:
                        tagBytes = 12 + 16;
                        break;
                    case 3:
                        tagBytes = 12 + 20;
                        break;
                    case 4:
                        tagBytes = 12 + 28;
                        break;
                    default:
                        SkColorSpacePrintf("Invalid parametric curve type\n");
                        return false;
                }
                break;
            default:
                SkColorSpacePrintf("Unsupported gamma tag type %d\n", type);
                return false;
        }

        // Adjust src and len if there is another gamma curve to load.
        if (0 != numGammas) {
            // Each curve is padded to 4-byte alignment.
            tagBytes = SkAlign4(tagBytes);
            if (len < tagBytes) {
                return false;
            }

            src += tagBytes;
            len -= tagBytes;
        }
    }

    // If all of the gammas we encounter are 1.0f, indicate that we failed to load gammas.
    // There is no need to apply a gamma of 1.0f.
    for (uint32_t i = 0; i < numGammas; i++) {
        if (1.0f != gammas[i]) {
            return true;
        }
    }

    return false;
}

static const uint32_t kTAG_AtoBType = SkSetFourByteTag('m', 'A', 'B', ' ');

bool load_color_lut(SkColorLookUpTable* colorLUT, uint32_t inputChannels, uint32_t outputChannels,
        const uint8_t* src, size_t len) {
    if (len < 20) {
        SkColorSpacePrintf("Color LUT tag is too small (%d bytes).", len);
        return false;
    }

    SkASSERT(inputChannels <= SkColorLookUpTable::kMaxChannels &&
             outputChannels <= SkColorLookUpTable::kMaxChannels);
    colorLUT->fInputChannels = inputChannels;
    colorLUT->fOutputChannels = outputChannels;
    uint32_t numEntries = 1;
    for (uint32_t i = 0; i < inputChannels; i++) {
        colorLUT->fGridPoints[i] = src[i];
        numEntries *= src[i];
    }
    numEntries *= outputChannels;

    // Space is provided for a maximum of the 16 input channels.  Now we determine the precision
    // of the table values.
    uint8_t precision = src[16];
    switch (precision) {
        case 1: // 8-bit data
        case 2: // 16-bit data
            break;
        default:
            SkColorSpacePrintf("Color LUT precision must be 8-bit or 16-bit.\n", len);
            return false;
    }

    if (len < 20 + numEntries * precision) {
        SkColorSpacePrintf("Color LUT tag is too small (%d bytes).", len);
        return false;
    }

    // Movable struct colorLUT has ownership of fTable.
    colorLUT->fTable = std::unique_ptr<float[]>(new float[numEntries]);
    const uint8_t* ptr = src + 20;
    for (uint32_t i = 0; i < numEntries; i++, ptr += precision) {
        if (1 == precision) {
            colorLUT->fTable[i] = ((float) ptr[i]) / 255.0f;
        } else {
            colorLUT->fTable[i] = ((float) read_big_endian_short(ptr)) / 65535.0f;
        }
    }

    return true;
}

bool load_matrix(SkFloat3x3* toXYZ, SkFloat3* toXYZOffset, const uint8_t* src, size_t len) {
    if (len < 48) {
        SkColorSpacePrintf("Matrix tag is too small (%d bytes).", len);
        return false;
    }

    toXYZ->fMat[0] = SkFixedToFloat(read_big_endian_int(src));
    toXYZ->fMat[3] = SkFixedToFloat(read_big_endian_int(src + 4));
    toXYZ->fMat[6] = SkFixedToFloat(read_big_endian_int(src + 8));
    toXYZ->fMat[1] = SkFixedToFloat(read_big_endian_int(src + 12));
    toXYZ->fMat[4] = SkFixedToFloat(read_big_endian_int(src + 16));
    toXYZ->fMat[7] = SkFixedToFloat(read_big_endian_int(src + 20));
    toXYZ->fMat[2] = SkFixedToFloat(read_big_endian_int(src + 24));
    toXYZ->fMat[5] = SkFixedToFloat(read_big_endian_int(src + 28));
    toXYZ->fMat[8] = SkFixedToFloat(read_big_endian_int(src + 32));
    toXYZOffset->fVec[0] = SkFixedToFloat(read_big_endian_int(src + 36));
    toXYZOffset->fVec[1] = SkFixedToFloat(read_big_endian_int(src + 40));
    toXYZOffset->fVec[2] = SkFixedToFloat(read_big_endian_int(src + 44));
    return true;
}

bool load_a2b0(SkColorLookUpTable* colorLUT, SkFloat3* gamma, SkFloat3x3* toXYZ,
        SkFloat3* toXYZOffset, const uint8_t* src, size_t len) {
    if (len < 32) {
        SkColorSpacePrintf("A to B tag is too small (%d bytes).", len);
        return false;
    }

    uint32_t type = read_big_endian_uint(src);
    if (kTAG_AtoBType != type) {
        // FIXME (msarett): Need to support lut8Type and lut16Type.
        SkColorSpacePrintf("Unsupported A to B tag type.\n");
        return false;
    }

    // Read the number of channels.  The four bytes that we skipped are reserved and
    // must be zero.
    uint8_t inputChannels = src[8];
    uint8_t outputChannels = src[9];
    if (0 == inputChannels || inputChannels > SkColorLookUpTable::kMaxChannels ||
            0 < outputChannels || outputChannels > SkColorLookUpTable::kMaxChannels) {
        // The color LUT assumes that there are at most 16 input channels.  For RGB
        // profiles, output channels should be 3.
        SkColorSpacePrintf("Too many input or output channels in A to B tag.\n");
        return false;
    }

    // Read the offsets of each element in the A to B tag.  With the exception of A curves and
    // B curves (which we do not yet support), we will handle these elements in the order in
    // which they should be applied (rather than the order in which they occur in the tag).
    // If the offset is non-zero it indicates that the element is present.
    uint32_t offsetToACurves = read_big_endian_int(src + 28);
    uint32_t offsetToBCurves = read_big_endian_int(src + 12);
    if ((0 != offsetToACurves) || (0 != offsetToBCurves)) {
        // FIXME (msarett): Handle A and B curves.
        // Note that the A curve is technically required in order to have a color LUT.
        // However, all the A curves I have seen so far have are just placeholders that
        // don't actually transform the data.
        SkColorSpacePrintf("Ignoring A and/or B curve.  Output may be wrong.\n");
    }

    uint32_t offsetToColorLUT = read_big_endian_int(src + 24);
    if (0 != offsetToColorLUT && offsetToColorLUT < len) {
        if (!load_color_lut(colorLUT, inputChannels, outputChannels, src + offsetToColorLUT,
                len - offsetToColorLUT)) {
            SkColorSpacePrintf("Failed to read color LUT from A to B tag.\n");
        }
    }

    uint32_t offsetToMCurves = read_big_endian_int(src + 20);
    if (0 != offsetToMCurves && offsetToMCurves < len) {
        if (!load_gammas(gamma->fVec, outputChannels, src + offsetToMCurves, len - offsetToMCurves))
        {
            SkColorSpacePrintf("Failed to read M curves from A to B tag.\n");
        }
    }

    uint32_t offsetToMatrix = read_big_endian_int(src + 16);
    if (0 != offsetToMatrix && offsetToMatrix < len) {
        if (!load_matrix(toXYZ, toXYZOffset, src + offsetToMatrix, len - offsetToMatrix)) {
            SkColorSpacePrintf("Failed to read matrix from A to B tag.\n");
        }
    }

    return true;
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

    switch (header.fInputColorSpace) {
        case kRGB_ColorSpace: {
            // Recognize the rXYZ, gXYZ, and bXYZ tags.
            const ICCTag* r = ICCTag::Find(tags.get(), tagCount, kTAG_rXYZ);
            const ICCTag* g = ICCTag::Find(tags.get(), tagCount, kTAG_gXYZ);
            const ICCTag* b = ICCTag::Find(tags.get(), tagCount, kTAG_bXYZ);
            if (r && g && b) {
                SkFloat3x3 toXYZ;
                if (!load_xyz(&toXYZ.fMat[0], r->addr((const uint8_t*) base), r->fLength) ||
                    !load_xyz(&toXYZ.fMat[3], g->addr((const uint8_t*) base), g->fLength) ||
                    !load_xyz(&toXYZ.fMat[6], b->addr((const uint8_t*) base), b->fLength))
                {
                    return_null("Need valid rgb tags for XYZ space");
                }

                // It is not uncommon to see missing or empty gamma tags.  This indicates
                // that we should use unit gamma.
                SkFloat3 gamma {{ 1.0f, 1.0f, 1.0f }};
                r = ICCTag::Find(tags.get(), tagCount, kTAG_rTRC);
                g = ICCTag::Find(tags.get(), tagCount, kTAG_gTRC);
                b = ICCTag::Find(tags.get(), tagCount, kTAG_bTRC);
                if (!r ||
                    !load_gammas(&gamma.fVec[0], 1, r->addr((const uint8_t*) base), r->fLength))
                {
                    SkColorSpacePrintf("Failed to read R gamma tag.\n");
                }
                if (!g ||
                    !load_gammas(&gamma.fVec[1], 1, g->addr((const uint8_t*) base), g->fLength))
                {
                    SkColorSpacePrintf("Failed to read G gamma tag.\n");
                }
                if (!b ||
                    !load_gammas(&gamma.fVec[2], 1, b->addr((const uint8_t*) base), b->fLength))
                {
                    SkColorSpacePrintf("Failed to read B gamma tag.\n");
                }
                return SkColorSpace::NewRGB(toXYZ, gamma);
            }

            // Recognize color profile specified by A2B0 tag.
            const ICCTag* a2b0 = ICCTag::Find(tags.get(), tagCount, kTAG_A2B0);
            if (a2b0) {
                SkColorLookUpTable colorLUT;
                SkFloat3 gamma;
                SkFloat3x3 toXYZ;
                SkFloat3 toXYZOffset;
                if (!load_a2b0(&colorLUT, &gamma, &toXYZ, &toXYZOffset,
                        a2b0->addr((const uint8_t*) base), a2b0->fLength)) {
                    return_null("Failed to parse A2B0 tag");
                }

                return sk_sp<SkColorSpace>(new SkColorSpace(std::move(colorLUT), gamma, toXYZ,
                                                            toXYZOffset));
            }

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
