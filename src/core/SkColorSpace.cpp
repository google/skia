/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkColorSpace.h"
#include "SkOncePtr.h"

static bool color_space_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.01f;
}

void SkFloat3::dump() const {
    SkDebugf("[%7.4f %7.4f %7.4f]\n", fVec[0], fVec[1], fVec[2]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static int32_t gUniqueColorSpaceID;

SkColorSpace::SkColorSpace(SkGammas gammas, const SkMatrix44& toXYZD50, Named named)
    : fGammas(std::move(gammas))
    , fToXYZD50(toXYZD50)
    , fUniqueID(sk_atomic_inc(&gUniqueColorSpaceID))
    , fNamed(named)
{}

SkColorSpace::SkColorSpace(SkColorLookUpTable colorLUT, SkGammas gammas,
                           const SkMatrix44& toXYZD50)
    : fColorLUT(std::move(colorLUT))
    , fGammas(std::move(gammas))
    , fToXYZD50(toXYZD50)
    , fUniqueID(sk_atomic_inc(&gUniqueColorSpaceID))
    , fNamed(kUnknown_Named)
{}

const float gSRGB_toXYZD50[] {
    0.4358f, 0.2224f, 0.0139f,    // * R
    0.3853f, 0.7170f, 0.0971f,    // * G
    0.1430f, 0.0606f, 0.7139f,    // * B
};

SK_DECLARE_STATIC_ONCE_PTR(SkColorSpace, sRGB);

sk_sp<SkColorSpace> SkColorSpace::NewRGB(SkGammas gammas, const SkMatrix44& toXYZD50) {
    // Check if we really have sRGB
    if (color_space_almost_equal(2.2f, gammas.fRed.fValue) &&
        color_space_almost_equal(2.2f, gammas.fGreen.fValue) &&
        color_space_almost_equal(2.2f, gammas.fBlue.fValue) &&
        color_space_almost_equal(toXYZD50.getFloat(0, 0), gSRGB_toXYZD50[0]) &&
        color_space_almost_equal(toXYZD50.getFloat(0, 1), gSRGB_toXYZD50[1]) &&
        color_space_almost_equal(toXYZD50.getFloat(0, 2), gSRGB_toXYZD50[2]) &&
        color_space_almost_equal(toXYZD50.getFloat(1, 0), gSRGB_toXYZD50[3]) &&
        color_space_almost_equal(toXYZD50.getFloat(1, 1), gSRGB_toXYZD50[4]) &&
        color_space_almost_equal(toXYZD50.getFloat(1, 2), gSRGB_toXYZD50[5]) &&
        color_space_almost_equal(toXYZD50.getFloat(2, 0), gSRGB_toXYZD50[6]) &&
        color_space_almost_equal(toXYZD50.getFloat(2, 1), gSRGB_toXYZD50[7]) &&
        color_space_almost_equal(toXYZD50.getFloat(2, 2), gSRGB_toXYZD50[8]) &&
        color_space_almost_equal(toXYZD50.getFloat(0, 3), 0.0f) &&
        color_space_almost_equal(toXYZD50.getFloat(1, 3), 0.0f) &&
        color_space_almost_equal(toXYZD50.getFloat(2, 3), 0.0f) &&
        color_space_almost_equal(toXYZD50.getFloat(3, 0), 0.0f) &&
        color_space_almost_equal(toXYZD50.getFloat(3, 1), 0.0f) &&
        color_space_almost_equal(toXYZD50.getFloat(3, 2), 0.0f) &&
        color_space_almost_equal(toXYZD50.getFloat(3, 3), 1.0f))
    {
        return SkColorSpace::NewNamed(kSRGB_Named);
    }

    return sk_sp<SkColorSpace>(new SkColorSpace(std::move(gammas), toXYZD50, kUnknown_Named));
}

sk_sp<SkColorSpace> SkColorSpace::NewNamed(Named named) {
    switch (named) {
        case kSRGB_Named: {
            SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
            srgbToxyzD50.set3x3ColMajorf(gSRGB_toXYZD50);
            return sk_ref_sp(sRGB.get([=]{
                return new SkColorSpace(SkGammas(2.2f, 2.2f, 2.2f), srgbToxyzD50, kSRGB_Named);
            }));
        }
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

bool SkColorSpace::LoadGammas(SkGammaCurve* gammas, uint32_t numGammas, const uint8_t* src,
                              size_t len) {
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
                    gammas[i].fValue = 1.0f;
                    break;
                } else if (len < 12 + 2 * count) {
                    SkColorSpacePrintf("gamma tag is too small (%d bytes)", len);
                    return false;
                }

                const uint16_t* table = (const uint16_t*) (src + 12);
                if (1 == count) {
                    // The table entry is the gamma (with a bias of 256).
                    uint16_t value = read_big_endian_short((const uint8_t*) table);
                    gammas[i].fValue = value / 256.0f;
                    SkColorSpacePrintf("gamma %d %g\n", value, *gamma);
                    break;
                }

                // Fill in the interpolation table.
                // FIXME (msarett):
                // We should recognize commonly occurring tables and just set gamma to 2.2f.
                gammas[i].fTableSize = count;
                gammas[i].fTable = std::unique_ptr<float[]>(new float[count]);
                for (uint32_t j = 0; j < count; j++) {
                    gammas[i].fTable[j] =
                            (read_big_endian_short((const uint8_t*) &table[j])) / 65535.0f;
                }
                break;
            }
            case kTAG_ParaCurveType:
                // Guess 2.2f.
                // FIXME (msarett): Handle parametric curves.
                SkColorSpacePrintf("parametric curve\n");
                gammas[i].fValue = 2.2f;

                // Determine the size of the parametric curve tag.
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

    return true;
}

static const uint32_t kTAG_AtoBType = SkSetFourByteTag('m', 'A', 'B', ' ');

bool SkColorSpace::LoadColorLUT(SkColorLookUpTable* colorLUT, uint32_t inputChannels,
                                uint32_t outputChannels, const uint8_t* src, size_t len) {
    if (len < 20) {
        SkColorSpacePrintf("Color LUT tag is too small (%d bytes).", len);
        return false;
    }

    SkASSERT(inputChannels <= SkColorLookUpTable::kMaxChannels && 3 == outputChannels);
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

bool load_matrix(SkMatrix44* toXYZ, const uint8_t* src, size_t len) {
    if (len < 48) {
        SkColorSpacePrintf("Matrix tag is too small (%d bytes).", len);
        return false;
    }

    float array[16];
    array[ 0] = SkFixedToFloat(read_big_endian_int(src));
    array[ 1] = SkFixedToFloat(read_big_endian_int(src + 4));
    array[ 2] = SkFixedToFloat(read_big_endian_int(src + 8));
    array[ 3] = 0;
    array[ 4] = SkFixedToFloat(read_big_endian_int(src + 12));
    array[ 5] = SkFixedToFloat(read_big_endian_int(src + 16));
    array[ 6] = SkFixedToFloat(read_big_endian_int(src + 20));
    array[ 7] = 0;
    array[ 8] = SkFixedToFloat(read_big_endian_int(src + 24));
    array[ 9] = SkFixedToFloat(read_big_endian_int(src + 28));
    array[10] = SkFixedToFloat(read_big_endian_int(src + 32));
    array[11] = 0;
    array[12] = SkFixedToFloat(read_big_endian_int(src + 36));  // translate R
    array[13] = SkFixedToFloat(read_big_endian_int(src + 40));  // translate G
    array[14] = SkFixedToFloat(read_big_endian_int(src + 44));
    array[15] = 1;
    toXYZ->setColMajorf(array);
    return true;
}

bool SkColorSpace::LoadA2B0(SkColorLookUpTable* colorLUT, SkGammas* gammas, SkMatrix44* toXYZ,
                            const uint8_t* src, size_t len) {
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
            3 != outputChannels) {
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
        if (!SkColorSpace::LoadColorLUT(colorLUT, inputChannels, outputChannels,
                                        src + offsetToColorLUT, len - offsetToColorLUT)) {
            SkColorSpacePrintf("Failed to read color LUT from A to B tag.\n");
        }
    }

    uint32_t offsetToMCurves = read_big_endian_int(src + 20);
    if (0 != offsetToMCurves && offsetToMCurves < len) {
        if (!SkColorSpace::LoadGammas(&gammas->fRed, outputChannels, src + offsetToMCurves,
                                      len - offsetToMCurves)) {
            SkColorSpacePrintf("Failed to read M curves from A to B tag.\n");
        }
    }

    uint32_t offsetToMatrix = read_big_endian_int(src + 16);
    if (0 != offsetToMatrix && offsetToMatrix < len) {
        if (!load_matrix(toXYZ, src + offsetToMatrix, len - offsetToMatrix)) {
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
                float toXYZ[9];
                if (!load_xyz(&toXYZ[0], r->addr((const uint8_t*) base), r->fLength) ||
                    !load_xyz(&toXYZ[3], g->addr((const uint8_t*) base), g->fLength) ||
                    !load_xyz(&toXYZ[6], b->addr((const uint8_t*) base), b->fLength))
                {
                    return_null("Need valid rgb tags for XYZ space");
                }

                // It is not uncommon to see missing or empty gamma tags.  This indicates
                // that we should use unit gamma.
                SkGammas gammas;
                r = ICCTag::Find(tags.get(), tagCount, kTAG_rTRC);
                g = ICCTag::Find(tags.get(), tagCount, kTAG_gTRC);
                b = ICCTag::Find(tags.get(), tagCount, kTAG_bTRC);
                if (!r || !SkColorSpace::LoadGammas(&gammas.fRed, 1,
                                                    r->addr((const uint8_t*) base), r->fLength)) {
                    SkColorSpacePrintf("Failed to read R gamma tag.\n");
                }
                if (!g || !SkColorSpace::LoadGammas(&gammas.fGreen, 1,
                                                    g->addr((const uint8_t*) base), g->fLength)) {
                    SkColorSpacePrintf("Failed to read G gamma tag.\n");
                }
                if (!b || !SkColorSpace::LoadGammas(&gammas.fBlue, 1,
                                                    b->addr((const uint8_t*) base), b->fLength)) {
                    SkColorSpacePrintf("Failed to read B gamma tag.\n");
                }

                SkMatrix44 mat(SkMatrix44::kUninitialized_Constructor);
                mat.set3x3ColMajorf(toXYZ);
                return SkColorSpace::NewRGB(std::move(gammas), mat);
            }

            // Recognize color profile specified by A2B0 tag.
            const ICCTag* a2b0 = ICCTag::Find(tags.get(), tagCount, kTAG_A2B0);
            if (a2b0) {
                SkColorLookUpTable colorLUT;
                SkGammas gammas;
                SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
                if (!SkColorSpace::LoadA2B0(&colorLUT, &gammas, &toXYZ,
                                            a2b0->addr((const uint8_t*) base), a2b0->fLength)) {
                    return_null("Failed to parse A2B0 tag");
                }

                // If there is no colorLUT, use NewRGB.  This allows us to check if the
                // profile is sRGB.
                if (!colorLUT.fTable) {
                    return SkColorSpace::NewRGB(std::move(gammas), toXYZ);
                }

                return sk_sp<SkColorSpace>(new SkColorSpace(std::move(colorLUT), std::move(gammas),
                                                            toXYZ));
            }

        }
        default:
            break;
    }

    return_null("ICC profile contains unsupported colorspace");
}
