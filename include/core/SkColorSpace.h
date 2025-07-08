/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_DEFINED
#define SkColorSpace_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkOnce.h"
#include "modules/skcms/skcms.h"

#include <cstddef>
#include <cstdint>

class SkData;

/**
 *  Describes a color gamut with primaries and a white point.
 */
struct SK_API SkColorSpacePrimaries {
    float fRX;
    float fRY;
    float fGX;
    float fGY;
    float fBX;
    float fBY;
    float fWX;
    float fWY;

    /**
     *  Convert primaries and a white point to a toXYZD50 matrix, the preferred color gamut
     *  representation of SkColorSpace.
     */
    bool toXYZD50(skcms_Matrix3x3* toXYZD50) const;
};

namespace SkNamedPrimaries {

////////////////////////////////////////////////////////////////////////////////
// Color primaries defined by ITU-T H.273, table 2. Names are given by the first
// specification referenced in the value's row.

// Rec. ITU-R BT.709-6, value 1.
static constexpr SkColorSpacePrimaries kRec709 = {
        0.64f, 0.33f, 0.3f, 0.6f, 0.15f, 0.06f, 0.3127f, 0.329f};

// Rec. ITU-R BT.470-6 System M (historical), value 4.
static constexpr SkColorSpacePrimaries kRec470SystemM = {
        0.67f, 0.33f, 0.21f, 0.71f, 0.14f, 0.08f, 0.31f, 0.316f};

// Rec. ITU-R BT.470-6 System B, G (historical), value 5.
static constexpr SkColorSpacePrimaries kRec470SystemBG = {
        0.64f, 0.33f, 0.29f, 0.60f, 0.15f, 0.06f, 0.3127f, 0.3290f};

// Rec. ITU-R BT.601-7 525, value 6.
static constexpr SkColorSpacePrimaries kRec601 = {
        0.630f, 0.340f, 0.310f, 0.595f, 0.155f, 0.070f, 0.3127f, 0.3290f};

// SMPTE ST 240, value 7 (functionally the same as value 6).
static constexpr SkColorSpacePrimaries kSMPTE_ST_240 = kRec601;

// Generic film (colour filters using Illuminant C), value 8.
static constexpr SkColorSpacePrimaries kGenericFilm = {
        0.681f, 0.319f, 0.243f, 0.692f, 0.145f, 0.049f, 0.310f, 0.316f};

// Rec. ITU-R BT.2020-2, value 9.
static constexpr SkColorSpacePrimaries kRec2020{
        0.708f, 0.292f, 0.170f, 0.797f, 0.131f, 0.046f, 0.3127f, 0.3290f};

// SMPTE ST 428-1, value 10.
static constexpr SkColorSpacePrimaries kSMPTE_ST_428_1 = {
        1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f / 3.f, 1.f / 3.f};

// SMPTE RP 431-2, value 11.
static constexpr SkColorSpacePrimaries kSMPTE_RP_431_2 = {
        0.680f, 0.320f, 0.265f, 0.690f, 0.150f, 0.060f, 0.314f, 0.351f};

// SMPTE EG 432-1, value 12.
static constexpr SkColorSpacePrimaries kSMPTE_EG_432_1 = {
        0.680f, 0.320f, 0.265f, 0.690f, 0.150f, 0.060f, 0.3127f, 0.3290f};

// No corresponding industry specification identified, value 22.
// This is sometimes referred to as EBU 3213-E, but that document doesn't
// specify these values.
static constexpr SkColorSpacePrimaries kITU_T_H273_Value22 = {
        0.630f, 0.340f, 0.295f, 0.605f, 0.155f, 0.077f, 0.3127f, 0.3290f};

// Mapping between names of color primaries and the number of the corresponding
// row in ITU-T H.273, table 2.  As above, the constants are named based on the
// first specification referenced in the value's row.
enum class CicpId : uint8_t {
    // Value 0 is reserved.
    kRec709 = 1,
    // Value 2 is unspecified.
    // Value 3 is reserved.
    kRec470SystemM = 4,
    kRec470SystemBG = 5,
    kRec601 = 6,
    kSMPTE_ST_240 = 7,
    kGenericFilm = 8,
    kRec2020 = 9,
    kSMPTE_ST_428_1 = 10,
    kSMPTE_RP_431_2 = 11,
    kSMPTE_EG_432_1 = 12,
    // Values 13-21 are reserved.
    kITU_T_H273_Value22 = 22,
    // Values 23-255 are reserved.
};

// https://www.w3.org/TR/css-color-4/#predefined-prophoto-rgb
static constexpr SkColorSpacePrimaries kProPhotoRGB = {
        0.7347f, 0.2653f, 0.1596f, 0.8404f, 0.0366f, 0.0001f, 0.34567f, 0.35850f};

}  // namespace SkNamedPrimaries

namespace SkNamedTransferFn {

// Like SkNamedGamut::kSRGB, keeping this bitwise exactly the same as skcms makes things fastest.
static constexpr skcms_TransferFunction kSRGB =
    { 2.4f, (float)(1/1.055), (float)(0.055/1.055), (float)(1/12.92), 0.04045f, 0.0f, 0.0f };

static constexpr skcms_TransferFunction k2Dot2 =
    { 2.2f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

static constexpr skcms_TransferFunction kRec2020 = {
        2.22222f, 0.909672f, 0.0903276f, 0.222222f, 0.0812429f, 0, 0};

////////////////////////////////////////////////////////////////////////////////
// Color primaries defined by ITU-T H.273, table 3. Names are given by the first
// specification referenced in the value's row.

// Rec. ITU-R BT.709-6, value 1.
static constexpr skcms_TransferFunction kRec709 = {2.222222222222f,
                                                   0.909672415686f,
                                                   0.090327584314f,
                                                   0.222222222222f,
                                                   0.081242858299f,
                                                   0.f,
                                                   0.f};

// Rec. ITU-R BT.470-6 System M (historical) assumed display gamma 2.2, value 4.
static constexpr skcms_TransferFunction kRec470SystemM = {2.2f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f};

// Rec. ITU-R BT.470-6 System B, G (historical) assumed display gamma 2.8,
// value 5.
static constexpr skcms_TransferFunction kRec470SystemBG = {2.8f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f};

// Rec. ITU-R BT.601-7, same as kRec709, value 6.
static constexpr skcms_TransferFunction kRec601 = kRec709;

// SMPTE ST 240, value 7.
static constexpr skcms_TransferFunction kSMPTE_ST_240 = {
        2.222222222222f, 0.899626676224f, 0.100373323776f, 0.25f, 0.091286342118f, 0.f, 0.f};

// Linear, value 8
static constexpr skcms_TransferFunction kLinear =
    { 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

// IEC 61966-2-4, value 11, same as kRec709 (but is explicitly extended).
static constexpr skcms_TransferFunction kIEC61966_2_4 = kRec709;

// IEC 61966-2-1 sRGB, value 13.
static constexpr skcms_TransferFunction kIEC61966_2_1 = kSRGB;

// Rec. ITU-R BT.2020-2 (10-bit system), value 14.
static constexpr skcms_TransferFunction kRec2020_10bit = kRec709;

// Rec. ITU-R BT.2020-2 (12-bit system), value 15.
static constexpr skcms_TransferFunction kRec2020_12bit = kRec709;

// Rec. ITU-R BT.2100-2 perceptual quantization (PQ) system, value 16.
static constexpr skcms_TransferFunction kPQ =
    {-2.0f, -107/128.0f, 1.0f, 32/2523.0f, 2413/128.0f, -2392/128.0f, 8192/1305.0f };

// SMPTE ST 428-1, value 17.
static constexpr skcms_TransferFunction kSMPTE_ST_428_1 = {
        2.6f, 1.034080527699f, 0.f, 0.f, 0.f, 0.f, 0.f};

// Rec. ITU-R BT.2100-2 hybrid log-gamma (HLG) system, value 18.
static constexpr skcms_TransferFunction kHLG =
    {-3.0f, 2.0f, 2.0f, 1/0.17883277f, 0.28466892f, 0.55991073f, 0.0f };

// Mapping between transfer function names and the number of the corresponding
// row in ITU-T H.273, table 3.  As above, the constants are named based on the
// first specification referenced in the value's row.
enum class CicpId : uint8_t {
    // Value 0 is reserved.
    kRec709 = 1,
    // Value 2 is unspecified.
    // Value 3 is reserved.
    kRec470SystemM = 4,
    kRec470SystemBG = 5,
    kRec601 = 6,
    kSMPTE_ST_240 = 7,
    kLinear = 8,
    // Value 9 is not supported by `SkColorSpace::MakeCICP`.
    // Value 10 is not supported by `SkColorSpace::MakeCICP`.
    kIEC61966_2_4 = 11,
    // Value 12 is not supported by `SkColorSpace::MakeCICP`.
    kIEC61966_2_1 = 13,
    kSRGB = kIEC61966_2_1,
    kRec2020_10bit = 14,
    kRec2020_12bit = 15,
    kPQ = 16,
    kSMPTE_ST_428_1 = 17,
    kHLG = 18,
    // Values 19-255 are reserved.
};

// https://w3.org/TR/css-color-4/#valdef-color-prophoto-rgb
// "The transfer curve is a gamma function with a value of 1/1.8"
static constexpr skcms_TransferFunction kProPhotoRGB = {1.8f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

// https://www.w3.org/TR/css-color-4/#predefined-a98-rgb
static constexpr skcms_TransferFunction kA98RGB = k2Dot2;

}  // namespace SkNamedTransferFn

namespace SkNamedGamut {

static constexpr skcms_Matrix3x3 kSRGB = {{
    // ICC fixed-point (16.16) representation, taken from skcms. Please keep them exactly in sync.
    // 0.436065674f, 0.385147095f, 0.143066406f,
    // 0.222488403f, 0.716873169f, 0.060607910f,
    // 0.013916016f, 0.097076416f, 0.714096069f,
    { SkFixedToFloat(0x6FA2), SkFixedToFloat(0x6299), SkFixedToFloat(0x24A0) },
    { SkFixedToFloat(0x38F5), SkFixedToFloat(0xB785), SkFixedToFloat(0x0F84) },
    { SkFixedToFloat(0x0390), SkFixedToFloat(0x18DA), SkFixedToFloat(0xB6CF) },
}};

static constexpr skcms_Matrix3x3 kAdobeRGB = {{
    // ICC fixed-point (16.16) repesentation of:
    // 0.60974, 0.20528, 0.14919,
    // 0.31111, 0.62567, 0.06322,
    // 0.01947, 0.06087, 0.74457,
    { SkFixedToFloat(0x9c18), SkFixedToFloat(0x348d), SkFixedToFloat(0x2631) },
    { SkFixedToFloat(0x4fa5), SkFixedToFloat(0xa02c), SkFixedToFloat(0x102f) },
    { SkFixedToFloat(0x04fc), SkFixedToFloat(0x0f95), SkFixedToFloat(0xbe9c) },
}};

static constexpr skcms_Matrix3x3 kDisplayP3 = {{
    {  0.515102f,   0.291965f,  0.157153f  },
    {  0.241182f,   0.692236f,  0.0665819f },
    { -0.00104941f, 0.0418818f, 0.784378f  },
}};

static constexpr skcms_Matrix3x3 kRec2020 = {{
    {  0.673459f,   0.165661f,  0.125100f  },
    {  0.279033f,   0.675338f,  0.0456288f },
    { -0.00193139f, 0.0299794f, 0.797162f  },
}};

static constexpr skcms_Matrix3x3 kXYZ = {{
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
}};

}  // namespace SkNamedGamut

class SK_API SkColorSpace : public SkNVRefCnt<SkColorSpace> {
public:
    /**
     *  Create the sRGB color space.
     */
    static sk_sp<SkColorSpace> MakeSRGB();

    /**
     *  Colorspace with the sRGB primaries, but a linear (1.0) gamma.
     */
    static sk_sp<SkColorSpace> MakeSRGBLinear();

    /**
     *  Create an SkColorSpace from a transfer function and a row-major 3x3 transformation to XYZ.
     */
    static sk_sp<SkColorSpace> MakeRGB(const skcms_TransferFunction& transferFn,
                                       const skcms_Matrix3x3& toXYZ);

    /**
     *  Create an SkColorSpace from code points specified in Rec. ITU-T H.273.
     *  Null will be returned for invalid or unsupported combination of code
     *  points.
     *
     *  Parameters:
     *
     * - `color_primaries` identifies an entry in Rec. ITU-T H.273, Table 2.
     * - `transfer_characteristics` identifies an entry in Rec. ITU-T H.273, Table 3.
     *
     * `SkColorSpace` (and the underlying `skcms_ICCProfile`) only supports RGB
     * color spaces and therefore this function does not take a
     * `matrix_coefficients` parameter - the caller is expected to verify that
     * `matrix_coefficients` is `0`.
     *
     * Narrow range images are extremely rare - see
     * https://github.com/w3c/png/issues/312#issuecomment-2327349614.  Therefore
     * this function doesn't take a `video_full_range_flag` - the caller is
     * expected to verify that it is `1` (indicating a full range image).
     */
    static sk_sp<SkColorSpace> MakeCICP(SkNamedPrimaries::CicpId color_primaries,
                                        SkNamedTransferFn::CicpId transfer_characteristics);

    /**
     *  Create an SkColorSpace from a parsed (skcms) ICC profile.
     */
    static sk_sp<SkColorSpace> Make(const skcms_ICCProfile&);

    /**
     *  Convert this color space to an skcms ICC profile struct.
     */
    void toProfile(skcms_ICCProfile*) const;

    /**
     *  Returns true if the color space gamma is near enough to be approximated as sRGB.
     */
    bool gammaCloseToSRGB() const;

    /**
     *  Returns true if the color space gamma is linear.
     */
    bool gammaIsLinear() const;

    /**
     *  Sets |fn| to the transfer function from this color space. Returns true if the transfer
     *  function can be represented as coefficients to the standard ICC 7-parameter equation.
     *  Returns false otherwise (eg, PQ, HLG).
     */
    bool isNumericalTransferFn(skcms_TransferFunction* fn) const;

    /**
     *  Returns true and sets |toXYZD50|.
     */
    bool toXYZD50(skcms_Matrix3x3* toXYZD50) const;

    /**
     *  Returns a hash of the gamut transformation to XYZ D50. Allows for fast equality checking
     *  of gamuts, at the (very small) risk of collision.
     */
    uint32_t toXYZD50Hash() const { return fToXYZD50Hash; }

    /**
     *  Returns a color space with the same gamut as this one, but with a linear gamma.
     */
    sk_sp<SkColorSpace> makeLinearGamma() const;

    /**
     *  Returns a color space with the same gamut as this one, but with the sRGB transfer
     *  function.
     */
    sk_sp<SkColorSpace> makeSRGBGamma() const;

    /**
     *  Returns a color space with the same transfer function as this one, but with the primary
     *  colors rotated. In other words, this produces a new color space that maps RGB to GBR
     *  (when applied to a source), and maps RGB to BRG (when applied to a destination).
     *
     *  This is used for testing, to construct color spaces that have severe and testable behavior.
     */
    sk_sp<SkColorSpace> makeColorSpin() const;

    /**
     *  Returns true if the color space is sRGB.
     *  Returns false otherwise.
     *
     *  This allows a little bit of tolerance, given that we might see small numerical error
     *  in some cases: converting ICC fixed point to float, converting white point to D50,
     *  rounding decisions on transfer function and matrix.
     *
     *  This does not consider a 2.2f exponential transfer function to be sRGB. While these
     *  functions are similar (and it is sometimes useful to consider them together), this
     *  function checks for logical equality.
     */
    bool isSRGB() const;

    /**
     *  Returns a serialized representation of this color space.
     */
    sk_sp<SkData> serialize() const;

    /**
     *  If |memory| is nullptr, returns the size required to serialize.
     *  Otherwise, serializes into |memory| and returns the size.
     */
    size_t writeToMemory(void* memory) const;

    static sk_sp<SkColorSpace> Deserialize(const void* data, size_t length);

    /**
     *  If both are null, we return true. If one is null and the other is not, we return false.
     *  If both are non-null, we do a deeper compare.
     */
    static bool Equals(const SkColorSpace*, const SkColorSpace*);

    void       transferFn(float gabcdef[7]) const;  // DEPRECATED: Remove when webview usage is gone
    void       transferFn(skcms_TransferFunction* fn) const;
    void    invTransferFn(skcms_TransferFunction* fn) const;
    void gamutTransformTo(const SkColorSpace* dst, skcms_Matrix3x3* src_to_dst) const;

    uint32_t transferFnHash() const { return fTransferFnHash; }
    uint64_t           hash() const { return (uint64_t)fTransferFnHash << 32 | fToXYZD50Hash; }

private:
    friend class SkColorSpaceSingletonFactory;

    SkColorSpace(const skcms_TransferFunction& transferFn, const skcms_Matrix3x3& toXYZ);

    void computeLazyDstFields() const;

    uint32_t                            fTransferFnHash;
    uint32_t                            fToXYZD50Hash;

    skcms_TransferFunction              fTransferFn;
    skcms_Matrix3x3                     fToXYZD50;

    mutable skcms_TransferFunction      fInvTransferFn;
    mutable skcms_Matrix3x3             fFromXYZD50;
    mutable SkOnce                      fLazyDstFieldsOnce;
};

#endif
