/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColorType.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkFloatBits.h"
#include "src/base/SkHalf.h"
#include "src/core/SkRasterPipeline.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureFormatXferFn.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "tools/ToolUtils.h"

// Uncomment the SK_ABORT() to exit on first pixel mismatch to help debugging
#define STOP_ON_TRANSFER_FAILURE // SK_ABORT();

namespace skgpu::graphite {

// Types for defining color type and format behavior expectations
enum ChannelDataType {
    UNorm, Signed, Float, FNorm, sRGB, XR, Pad
};

struct Channel {
    char fName; // rgbayuv01 and G (for gray)
    int fBits;
    ChannelDataType fType;
};

struct ColorTypeExpectation {
    SkColorType fColorType;
    Swizzle fReadSwizzle;
    std::optional<Swizzle> fWriteSwizzle; // not set implies not renderable
};

struct FormatExpectation {
    TextureFormat fFormat;
    skia_private::TArray<Channel> fChannels;
    SkTextureCompressionType fCompressionType = SkTextureCompressionType::kNone;

    // Not set implies transfers are disabled; it is composed with a ColorTypeExpectation's
    // read or write swizzle to produce the expected readback/upload swizzle.
    std::optional<Swizzle> fXferSwizzle;
    // The first color type expectation is assumed to be the best fit.
    skia_private::TArray<ColorTypeExpectation> fCompatibleColorTypes;

    // All of the expectations for the fixed properties of a TextureFormat are derived from its
    // fChannels definition.
    bool isFloatingPoint() const { return this->hasType(Float) || this->hasType(FNorm); }
    bool hasDepth() const { return this->hasChannel('d'); }
    bool hasStencil() const { return this->hasChannel('s'); }
    bool hasDepthOrStencil() const { return this->hasDepth() || this->hasStencil(); }
    bool isMultiplanar() const {
        return this->hasChannel('y') && this->hasChannel('u') && this->hasChannel('v');
    }

    bool autoClamps() const {
        // Auto clamping is derived from the type of the channel with the most bits
        bool autoClamp = true;
        int maxBitSize = 0;
        for (const Channel& c : fChannels) {
            if (c.fBits > maxBitSize) {
                autoClamp = c.fType == UNorm || c.fType == sRGB;
                maxBitSize = c.fBits;
            }
        }
        return autoClamp;
    }

    int bytesPerBlock() const {
        if (fCompressionType != SkTextureCompressionType::kNone) {
            // At the moment, all supported compression types have the same bytes per block
            return 8;
        }

        int bitCount = 0;
        for (const Channel& c : fChannels) {
            bitCount += c.fBits;
        }
        return bitCount / 8;
    }

    uint32_t channelMask() const {
        uint32_t mask = 0;
        if (this->hasChannel('r') || this->hasChannel('y')) {
            mask |= kRed_SkColorChannelFlag;
        }
        if (this->hasChannel('g') || this->hasChannel('u')) {
            mask |= kGreen_SkColorChannelFlag;
        }
        if (this->hasChannel('b') || this->hasChannel('v')) {
            mask |= kBlue_SkColorChannelFlag;
        }
        if (this->hasChannel('a')) {
            mask |= kAlpha_SkColorChannelFlag;
        }
        // Other channels do not contribute to SkColorChannel mask
        return mask;
    }

private:
    bool hasChannel(char channel) const {
        for (const Channel& c : fChannels) {
            if (c.fName == channel) {
                return true;
            }
        }
        return false;
    }
    bool hasType(ChannelDataType type) const {
        for (const Channel& c : fChannels) {
            if (c.fType == type) {
                return true;
            }
        }
        return false;
    }
};

using PixelData = std::array<uint8_t, 16>; // The largest texel/pixel size is RGBA32F = 16 bytes

uint32_t channel_to_bits(const Channel& channel, float value) {
    switch (channel.fType) {
        case Pad:
            // Pad should only be used with 'x', which produces NaN in channel_to_float, so
            // replace `value` with the default 'x' bit pattern in gen_channel_values, and then
            // fall through to UNorm handling to adjust it to the right bit depth
            SkASSERT(channel.fName == 'x');
            value = 0b0101 / 15.f;
            [[fallthrough]];

        case sRGB:
            // sRGB data is stored in a non-linear gamma and automatically decodes to linear when
            // being sampled or rendered into. This means an SRGB_8888 image with a linear
            // SkColorSpace behaves like a regular 8888 image with an sRGB SkColorSpace. But
            // transfers between CPU and GPU assume the SkColorSpace is the same, so we need to
            // interpret the `v` as if it were an 8888 image with a linear SkColorSpace and map to
            // the sRGB encoding.
            if (channel.fType == sRGB) {
                value = skcms_TransferFunction_eval(skcms_sRGB_Inverse_TransferFunction(), value);
            }
            [[fallthrough]];

        case UNorm:
            return (uint32_t) std::round(value * ((1 << channel.fBits) - 1));

        case XR:
            // See SkRP_opts::store_1010102_xr
            SkASSERT(channel.fBits == 10);
            return std::min((uint32_t) std::round(value * 510 + 384), 1023u);

        case FNorm:
            // For simplicity fall through to Float, the Norm is just a hint about range
            [[fallthrough]];
        case Float:
            SkASSERT(channel.fBits == 16 || channel.fBits == 32);
            return channel.fBits == 16 ? (uint32_t) SkFloatToHalf(value) : SkFloat2Bits(value);

        case Signed:
            // This should only be used for 's' channels, which are already not reaching this code
            SK_ABORT("Should not be generating Signed values for pixel data");
    }
    SkUNREACHABLE;
}

float channel_to_float(const Channel& channel, uint32_t bits) {
    switch (channel.fType) {
        case sRGB: [[fallthrough]]; // first treat as unorm then apply gamma TF
        case UNorm: {
            float vf = bits * (1 / (float) ((1 << channel.fBits) - 1));
            if (channel.fType == sRGB) {
                vf = skcms_TransferFunction_eval(skcms_sRGB_TransferFunction(), vf);
            }
            return vf;
        }

        case XR:     return (bits - 384.f) * (1/510.f);

        case FNorm:  [[fallthrough]]; // Values are interpreted the same, values are in [0,1]
        case Float:  return channel.fBits == 16 ? SkHalfToFloat((SkHalf) bits) : SkBits2Float(bits);

        case Signed: [[fallthrough]];
        case Pad:    return SK_FloatNaN; // No floating point printing
    }
    SkUNREACHABLE;
}

// Generate a unique value for `channel` by applying `srcToDst` to the source channels.
float gen_channel_value(const Channel& channel,
                        const Swizzle& srcToDst,
                        SkSpan<const Channel> srcChannels) {
    // First apply the swizzle
    char name = channel.fName;
    switch (name) {
        case 'r': name = srcToDst[0]; break;
        case 'g': name = srcToDst[1]; break;
        case 'b': name = srcToDst[2]; break;
        case 'a': name = srcToDst[3]; break;
    }

    // Try to look for an exact channel match, in which case we use the unique value corresponding
    // to that channel name.
    for (size_t c = 0; c < srcChannels.size(); ++c) {
        if (srcChannels[c].fName == name ||
            (srcChannels[c].fName == 'G' && (name == 'r' || name == 'g' || name == 'b'))) {
            uint32_t v;
            switch (srcChannels[c].fName) {
                case 'r': v = 0b0001; break;
                case 'g': v = 0b0010; break;
                case 'b': v = 0b0100; break;
                case 'a': v = 0b1010; break; // `A` must additionally fit into 2 bit channel formats
                case 'x': v = 0b0101; break;
                case '0': v = 0b0000; break;
                case '1': v = 0b1111; break;
                case 'G': v = 0b1100; break; // arbitrary starting Gray value
                default:
                    // NOTE: 'd', 's', 'y', 'u', 'v' are valid channel names, but the formats that
                    // have those should not be participating in the read/write transfer testing.
                    SK_ABORT("Bad source channel name for pixel data generation: %c", name);
            }
            // Route through the source channel's bit representation to account for rounding
            return channel_to_float(srcChannels[c], channel_to_bits(srcChannels[c], v / 15.f));
        }
    }

    // We didn't find an exact channel match, so we either use default values or apply gray handling
    switch (name) {
        // Trivial default values
        case 'x':                               return SK_FloatNaN;
        case 'r': case 'g': case 'b': case '0': return 0.f;
        case 'a': case '1':                     return 1.f;

        // Construct a gray value from r, g, and b values of the source
        case 'G': {
            float r = gen_channel_value({'r', channel.fBits, channel.fType}, srcToDst, srcChannels);
            float g = gen_channel_value({'g', channel.fBits, channel.fType}, srcToDst, srcChannels);
            float b = gen_channel_value({'b', channel.fBits, channel.fType}, srcToDst, srcChannels);

            return 0.2126f * r + 0.7142f * g + 0.0722f * b;
        }

        default:
            SK_ABORT("Bad dst channel name for pixel data generation: %c", name);
    }
}

// Src and Dst must be (uint32_t, PixelData) or (PixelData, uint32_t), and `dst` should be
// zero-initialized before calling. Returns the bitOffset for the next channel.
template <typename Src, typename Dst>
int copy_unaligned_bits(int numBits, int bitOffset, const Src& src, Dst& dst) {
    static_assert(std::is_same_v<Src, uint32_t> || std::is_same_v<Src, PixelData>);
    static_assert(std::is_same_v<Dst, uint32_t> || std::is_same_v<Dst, PixelData>);
    static_assert(!std::is_same_v<Src, Dst>);
    static constexpr bool kBytesToChannel = std::is_same_v<Src, PixelData>;

    int srcBitsLeft = numBits;
    while (srcBitsLeft > 0) {
        int arrayIndex = bitOffset / 8;
        int arrayBitsLeft = (arrayIndex + 1) * 8 - bitOffset;
        int copyBits = std::min(arrayBitsLeft, srcBitsLeft);

        int channelBitShift = numBits - srcBitsLeft;
        int arrayBitShift = 8 - arrayBitsLeft;

        if constexpr (kBytesToChannel) {
            dst |= ((src[arrayIndex] >> arrayBitShift) & ((1<<copyBits) - 1)) << channelBitShift;
        } else {
            dst[arrayIndex] |= ((src >> channelBitShift) & ((1<<copyBits) - 1)) << arrayBitShift;
        }
        srcBitsLeft -= copyBits;
        bitOffset += copyBits;
    }
    return bitOffset;
}

PixelData gen_pixel_data(SkSpan<const Channel> dstChannels,
                         Swizzle srcToDst,
                         SkSpan<const Channel> srcChannels) {
    PixelData pixel{}; // zero-initialize all bytes
    int bitOffset = 0;
    for (size_t c = 0; c < dstChannels.size(); ++c) {
        float srcValue = gen_channel_value(dstChannels[c], srcToDst, srcChannels);
        uint32_t channelValue = channel_to_bits(dstChannels[c], srcValue);
        bitOffset = copy_unaligned_bits(dstChannels[c].fBits, bitOffset, channelValue, pixel);
    }

    return pixel;
}

// No swizzling or data conversion variant for input data generation
PixelData gen_pixel_data(SkSpan<const Channel> channels) {
    return gen_pixel_data(channels, Swizzle::RGBA(), channels);
}

const char* channel_type_name(ChannelDataType type) {
    switch (type) {
        case UNorm:  return "u";
        case Signed: return "s";
        case Float:  return "f";
        case FNorm:  return "fn";
        case sRGB:   return "srgb";
        case XR:     return "xr";
        case Pad:    return "_";
    }
    SkUNREACHABLE;
}

skia_private::TArray<SkString> print_channel_names(SkSpan<const Channel> channels) {
    skia_private::TArray<SkString> names;
    names.push_back(SkString()); // empty to align with channel values row label
    names.push_back(SkString("raw"));
    for (const Channel& c : channels) {
        const char* typeName = channel_type_name(c.fType);
        names.push_back(SkStringPrintf("%c(%s%d)", c.fName, typeName, c.fBits));
    }
    return names;
}

skia_private::TArray<SkString> print_channel_values(
        const char* prefix, SkSpan<const Channel> channels, const PixelData& pixel) {
    skia_private::TArray<SkString> values;

    values.push_back(SkString(prefix));

    values.push_back(SkString()); // Raw hex value
    for (int i = 0; i < 16; ++i) {
        values[1].appendf(" %0.2x", pixel[i]);
    }

    int bitOffset = 0;
    for (const Channel& c : channels) {
        uint32_t channelValue = 0;
        bitOffset = copy_unaligned_bits(c.fBits, bitOffset, pixel, channelValue);

        SkString binary;
        for (int i = 0; i < c.fBits; ++i) {
            binary.prependUnichar((channelValue >> i) & 1 ? '1' : '0');
        }
        float vf = channel_to_float(c, channelValue);

        SkString numeric;
        switch (c.fType) {
            case Pad:    numeric = "-"; break;
            case Signed: numeric = SkStringPrintf("%u", channelValue); break;

            case sRGB:   [[fallthrough]];
            case XR:
            case UNorm:  numeric = SkStringPrintf("0x%x / %.2f", channelValue, vf); break;

            case FNorm:  [[fallthrough]];
            case Float:  numeric = SkStringPrintf("%0.4f", vf); break;
        }
        values.push_back(SkStringPrintf("%s (%s)", binary.c_str(), numeric.c_str()));
    }

    return values;
}

void dump_string(const SkString& str, int length) {
    int strLen = SkTo<int>(str.size());
    SkASSERT(strLen <= length);
    SkDebugf("%*s", std::min(length, strLen), str.data());
    length -= strLen;
    while(length > 0) {
        SkDebugf(" ");
        length--;
    }
}

void dump_pixel_comparison(const SkString& inputName,
                           SkSpan<const Channel> inputChannels,
                           const PixelData& inputPixel,
                           const SkString& outputName,
                           SkSpan<const Channel> outputChannels,
                           const PixelData& expectedOutputPixel,
                           const PixelData& actualOutputPixel) {
    SkDebugf("Transfer from %s to %s:\n", inputName.c_str(), outputName.c_str());

    auto inputChannelNames = print_channel_names(inputChannels);
    auto inputChannelValues = print_channel_values("Input", inputChannels, inputPixel);

    auto outputChannelNames = print_channel_names(outputChannels);
    auto expectedChannelValues =
            print_channel_values("Expected", outputChannels, expectedOutputPixel);
    auto actualChannelValues =
            print_channel_values("Actual", outputChannels, actualOutputPixel);

    const int colCount = std::max(inputChannelNames.size(), outputChannelNames.size());
    auto rows = {inputChannelNames,
                 inputChannelValues,
                 outputChannelNames,
                 expectedChannelValues,
                 actualChannelValues};
    for (auto&& row : rows) {
        for (int c = 0; c < std::min(colCount, row.size()); ++c) {
            if (c == 0) {
                SkDebugf(" ");
            } else {
                SkDebugf(" | ");
            }
            int colWidth = 0;
            for (auto&& otherRow : rows) {
                if (c < otherRow.size()) {
                    colWidth = std::max(colWidth, SkTo<int>(otherRow[c].size()));
                }
            }
            dump_string(row[c], colWidth);
        }
        SkDebugf("\n");
    }
}

int channel_tolerance(SkSpan<const Channel> dstChannels, SkSpan<const Channel> srcChannels) {
    bool srcHasSRGB = false;
    for (const Channel c : srcChannels) {
        srcHasSRGB |= c.fType == sRGB;
    }
    bool dstHasSRGBOrGray = false;
    for (const Channel c : dstChannels) {
         dstHasSRGBOrGray |= c.fType == sRGB || c.fName == 'G';
    }
    // If the input or output data involves sRGB encoding/decoding or conversion to gray, we
    // allow 1 bit of difference because of mismatches between how SkRasterPipeline calculates
    // channel values vs. the value generation in these tests.
    return srcHasSRGB || dstHasSRGBOrGray ? 1 : 0;
}

bool compare_pixels(SkSpan<const Channel> channels,
                    const PixelData& expected,
                    const PixelData& actual,
                    int channelTolerance) {
    int bitOffset = 0;
    for (const Channel& c : channels) {
        uint32_t actualChannelBits = 0;
        uint32_t expectedChannelBits = 0;
        copy_unaligned_bits(c.fBits, bitOffset, expected, expectedChannelBits);
        copy_unaligned_bits(c.fBits, bitOffset, actual, actualChannelBits);
        bitOffset += c.fBits;

        int64_t channelDiff = static_cast<int64_t>(actualChannelBits) -
                              static_cast<int64_t>(expectedChannelBits);
        if (c.fType != Pad && std::abs(channelDiff) > channelTolerance) {
            return false;
        }
    }
    return true;
}

PixelData transfer_data(const TextureFormatXferFn& xferFn, const PixelData& inputData) {
    PixelData outputData{}; // zero-initialize for comparison stability on unwritten values.
    xferFn.run(1, 1, &inputData, sizeof(PixelData), &outputData, sizeof(PixelData));
    return outputData;
}

// Define the channel layout for every SkColorType for use in generating and validating the
// result of transferring data to or from a texture format.
[[maybe_unused]] static const struct ColorTypeChannels {
    SkColorType fColorType;
    Swizzle fEffectiveSwizzle; // Derivable from channel mask
    skia_private::TArray<Channel> fChannels;
} kColorTypeChannels[] {
    {kAlpha_8_SkColorType,            Swizzle("000a"), {{'a', 8, UNorm}}},
    // NOTE: 565 and 4444 are misnamed and are BGR and ABGR respectively.
    {kRGB_565_SkColorType,            Swizzle("rgb1"), {{'b', 5, UNorm},
                                                        {'g', 6, UNorm},
                                                        {'r', 5, UNorm}}},
    {kARGB_4444_SkColorType,          Swizzle("rgba"), {{'a', 4, UNorm},
                                                        {'b', 4, UNorm},
                                                        {'g', 4, UNorm},
                                                        {'r', 4, UNorm}}},
    {kRGBA_8888_SkColorType,          Swizzle("rgba"), {{'r', 8, UNorm},
                                                        {'g', 8, UNorm},
                                                        {'b', 8, UNorm},
                                                        {'a', 8, UNorm}}},
    {kRGB_888x_SkColorType,           Swizzle("rgb1"), {{'r', 8, UNorm},
                                                        {'g', 8, UNorm},
                                                        {'b', 8, UNorm},
                                                        {'x', 8, Pad}}},
    {kBGRA_8888_SkColorType,          Swizzle("rgba"), {{'b', 8, UNorm},
                                                        {'g', 8, UNorm},
                                                        {'r', 8, UNorm},
                                                        {'a', 8, UNorm}}},
    {kRGBA_1010102_SkColorType,       Swizzle("rgba"), {{'r', 10, UNorm},
                                                        {'g', 10, UNorm},
                                                        {'b', 10, UNorm},
                                                        {'a', 2, UNorm}}},
    {kBGRA_1010102_SkColorType,       Swizzle("rgba"), {{'b', 10, UNorm},
                                                        {'g', 10, UNorm},
                                                        {'r', 10, UNorm},
                                                        {'a', 2, UNorm}}},
    {kRGB_101010x_SkColorType,        Swizzle("rgb1"), {{'r', 10, UNorm},
                                                        {'g', 10, UNorm},
                                                        {'b', 10, UNorm},
                                                        {'x', 2, Pad}}},
    {kBGR_101010x_SkColorType,        Swizzle("rgb1"), {{'b', 10, UNorm},
                                                        {'g', 10, UNorm},
                                                        {'r', 10, UNorm},
                                                        {'x', 2, Pad}}},
    {kBGR_101010x_XR_SkColorType,     Swizzle("rgb1"), {{'b', 10, XR},
                                                        {'g', 10, XR},
                                                        {'r', 10, XR},
                                                        {'x', 2, Pad}}},
    {kBGRA_10101010_XR_SkColorType,   Swizzle("rgba"), {{'x', 6, Pad}, {'b', 10, XR},
                                                        {'x', 6, Pad}, {'g', 10, XR},
                                                        {'x', 6, Pad}, {'r', 10, XR},
                                                        {'x', 6, Pad}, {'a', 10, XR}}},
    {kRGBA_10x6_SkColorType,          Swizzle("rgba"), {{'x', 6, Pad}, {'r', 10, UNorm},
                                                        {'x', 6, Pad}, {'g', 10, UNorm},
                                                        {'x', 6, Pad}, {'b', 10, UNorm},
                                                        {'x', 6, Pad}, {'a', 10, UNorm}}},
    // NOTE: The swizzle is rrr1 since we store gray in the red channel of the texture, but we use
    // 'G' as the channel to force generating gray/luminance values in the tests instead of just 'r'
    {kGray_8_SkColorType,             Swizzle("rrr1"), {{'G', 8, UNorm}}},
    {kRGBA_F16Norm_SkColorType,       Swizzle("rgba"), {{'r', 16, FNorm},
                                                        {'g', 16, FNorm},
                                                        {'b', 16, FNorm},
                                                        {'a', 16, FNorm}}},
    {kRGBA_F16_SkColorType,           Swizzle("rgba"), {{'r', 16, Float},
                                                        {'g', 16, Float},
                                                        {'b', 16, Float},
                                                        {'a', 16, Float}}},
    {kRGB_F16F16F16x_SkColorType,     Swizzle("rgb1"), {{'r', 16, Float},
                                                        {'g', 16, Float},
                                                        {'b', 16, Float},
                                                        {'x', 16, Pad}}},
    {kRGBA_F32_SkColorType,           Swizzle("rgba"), {{'r', 32, Float},
                                                        {'g', 32, Float},
                                                        {'b', 32, Float},
                                                        {'a', 32, Float}}},
    {kR8G8_unorm_SkColorType,         Swizzle("rg01"), {{'r', 8, UNorm}, {'g', 8, UNorm}}},
    {kA16_float_SkColorType,          Swizzle("000a"), {{'a', 16, Float}}},
    {kR16_float_SkColorType,          Swizzle("r001"), {{'r', 16, Float}}},
    {kR16G16_float_SkColorType,       Swizzle("rg01"), {{'r', 16, Float}, {'g', 16, Float}}},
    {kA16_unorm_SkColorType,          Swizzle("000a"), {{'a', 16, UNorm}}},
    {kR16_unorm_SkColorType,          Swizzle("r001"), {{'r', 16, UNorm}}},
    {kR16G16_unorm_SkColorType,       Swizzle("rg01"), {{'r', 16, UNorm}, {'g', 16, UNorm}}},
    {kR16G16B16A16_unorm_SkColorType, Swizzle("rgba"), {{'r', 16, UNorm},
                                                        {'g', 16, UNorm},
                                                        {'b', 16, UNorm},
                                                        {'a', 16, UNorm}}},
    {kSRGBA_8888_SkColorType,         Swizzle("rgba"), {{'r', 8, sRGB},
                                                        {'g', 8, sRGB},
                                                        {'b', 8, sRGB},
                                                        {'a', 8, UNorm}}},
    {kR8_unorm_SkColorType,           Swizzle("r001"), {{'r', 8, UNorm}}},
};
// Must include one per SkColorType except for kUnknown
static_assert(std::size(kColorTypeChannels) == kSkColorTypeCnt - 1,
              "Missing channel definition for SkColorType");

// Full definition of compatibility and behavior between SkColorType and TextureFormat:
static const FormatExpectation kExpectations[] {
    {.fFormat=TextureFormat::kUnsupported,
     .fChannels={},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kR8,
     .fChannels={{'r', 8, UNorm}},
     .fXferSwizzle=Swizzle("r001"),
     .fCompatibleColorTypes={{kR8_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kAlpha_8_SkColorType,  Swizzle("000r"), Swizzle("a000")},
                             {kGray_8_SkColorType,   Swizzle("rrra"), std::nullopt}}},

    {.fFormat=TextureFormat::kR16,
     .fChannels={{'r', 16, UNorm}},
     .fXferSwizzle=Swizzle("r001"),
     .fCompatibleColorTypes={{kR16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kA16_unorm_SkColorType, Swizzle("000r"), Swizzle("a000")}}},

    {.fFormat=TextureFormat::kR16F,
     .fChannels={{'r', 16, Float}},
     .fXferSwizzle=Swizzle("r001"),
     .fCompatibleColorTypes={{kR16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kA16_float_SkColorType, Swizzle("000r"), Swizzle("a000")}}},

    {.fFormat=TextureFormat::kR32F,
     .fChannels={{'r', 32, Float}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kR16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kA8,
     .fChannels={{'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("000a"),
     .fCompatibleColorTypes={{kAlpha_8_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRG8,
     .fChannels={{'r', 8, UNorm}, {'g', 8, UNorm}},
     .fXferSwizzle=Swizzle("rg01"),
     .fCompatibleColorTypes={{kR8G8_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRG16,
     .fChannels={{'r', 16, UNorm}, {'g', 16, UNorm}},
     .fXferSwizzle=Swizzle("rg01"),
     .fCompatibleColorTypes={{kR16G16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRG16F,
     .fChannels={{'r', 16, Float}, {'g', 16, Float}},
     .fXferSwizzle=Swizzle("rg01"),
     .fCompatibleColorTypes={{kR16G16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRG32F,
     .fChannels={{'r', 32, Float}, {'g', 32, Float}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kR16G16_float_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB8,
     .fChannels={{'r', 8, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kBGR8,
     .fChannels={{'b', 8, UNorm}, {'g', 8, UNorm}, {'r', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kB5_G6_R5,
     .fChannels={{'b', 5, UNorm}, {'g', 6, UNorm}, {'r', 5, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_565_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kR5_G6_B5,
     .fChannels={{'r', 5, UNorm}, {'g', 6, UNorm}, {'b', 5, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_565_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB16,
     .fChannels={{'r', 16, UNorm}, {'g', 16, UNorm}, {'b', 16, UNorm}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kR16G16B16A16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB16F,
     .fChannels={{'r', 16, Float}, {'g', 16, Float}, {'b', 16, Float}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGB_F16F16F16x_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB32F,
     .fChannels={{'r', 32, Float}, {'g', 32, Float}, {'b', 32, Float}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kRGBA_F32_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB8_sRGB,
     .fChannels={{'r', 8, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kBGR10_XR,
     .fChannels={{'b', 10, XR}, {'g', 10, XR}, {'r', 10, XR}, {'x', 2, Pad}},
     .fXferSwizzle=Swizzle("rgb1"),
     .fCompatibleColorTypes={{kBGR_101010x_XR_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGBA8,
     .fChannels={{'r', 8, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}, {'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kBGRA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_888x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA16,
     .fChannels={{'r', 16, UNorm}, {'g', 16, UNorm}, {'b', 16, UNorm}, {'a', 16, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kR16G16B16A16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGBA16F,
     .fChannels={{'r', 16, Float}, {'g', 16, Float}, {'b', 16, Float}, {'a', 16, Float}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_F16_SkColorType,       Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGBA_F16Norm_SkColorType,   Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_F16F16F16x_SkColorType, Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA32F,
     .fChannels={{'r', 32, Float}, {'g', 32, Float}, {'b', 32, Float}, {'a', 32, Float}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_F32_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGB10_A2,
     .fChannels={{'r', 10, UNorm}, {'g', 10, UNorm}, {'b', 10, UNorm}, {'a', 2, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_101010x_SkColorType,  Swizzle::RGB1(), std::nullopt},
                             {kBGRA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kBGR_101010x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA10x6,
     .fChannels={{'x', 6, Pad}, {'r', 10, UNorm}, {'x', 6, Pad}, {'g', 10, UNorm},
                 {'x', 6, Pad}, {'b', 10, UNorm}, {'x', 6, Pad}, {'a', 10, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kRGBA_10x6_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kRGBA8_sRGB,
     .fChannels={{'r', 8, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}, {'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kBGRA8,
     .fChannels={{'b', 8, UNorm}, {'g', 8, UNorm}, {'r', 8, UNorm}, {'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kBGRA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_888x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kBGR10_A2,
     .fChannels={{'b', 10, UNorm}, {'g', 10, UNorm}, {'r', 10, UNorm}, {'a', 2, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kBGRA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kBGR_101010x_SkColorType,  Swizzle::RGB1(), std::nullopt},
                             {kRGBA_1010102_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()},
                             {kRGB_101010x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kBGRA8_sRGB,
     .fChannels={{'b', 8, sRGB}, {'g', 8, sRGB}, {'r', 8, sRGB}, {'a', 8, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kABGR4,
     .fChannels={{'a', 4, UNorm}, {'b', 4, UNorm}, {'g', 4, UNorm}, {'r', 4, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kARGB_4444_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    {.fFormat=TextureFormat::kARGB4,
     // TODO(michaelludwig): kARGB_4444 color type is actually BGRA order. Historically, we
     // configured kARGB4 format to swizzle the channels on read and write in the shader so that the
     // CPU data could be uploaded directly. When we can perform a RB channel swap as part of
     // upload/readback, then this can change to RGBA swizzles.
     .fChannels={{'a', 4, UNorm}, {'r', 4, UNorm}, {'g', 4, UNorm}, {'b', 4, UNorm}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kARGB_4444_SkColorType, Swizzle::BGRA(), Swizzle::BGRA()}}},

    {.fFormat=TextureFormat::kBGRA10x6_XR,
     .fChannels={{'x', 6, Pad}, {'b', 10, XR}, {'x', 6, Pad}, {'g', 10, XR},
                 {'x', 6, Pad}, {'r', 10, XR}, {'x', 6, Pad}, {'a', 10, XR}},
     .fXferSwizzle=Swizzle("rgba"),
     .fCompatibleColorTypes={{kBGRA_10101010_XR_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}}},

    // For compressed formats, the bytes per block represents actual compressed block size, not
    // just the size of a pixel.
    {.fFormat=TextureFormat::kRGB8_ETC2,
     .fChannels={{'r', 10, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}},
     .fCompressionType=SkTextureCompressionType::kETC2_RGB8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGB8_ETC2_sRGB,
     .fChannels={{'r', 10, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}},
     .fCompressionType=SkTextureCompressionType::kETC2_RGB8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGB8_BC1,
     .fChannels={{'r', 10, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}},
     .fCompressionType=SkTextureCompressionType::kBC1_RGB8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA8_BC1,
     .fChannels={{'r', 10, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}, {'a', 8, UNorm}},
     .fCompressionType=SkTextureCompressionType::kBC1_RGBA8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt},
                             {kRGB_888x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kRGBA8_BC1_sRGB,
     .fChannels={{'r', 10, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}, {'a', 8, UNorm}},
     .fCompressionType=SkTextureCompressionType::kBC1_RGBA8_UNORM,
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kSRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    // For these multiplanar formats, we set the bytes per block assuming the UV planes are the
    // same size as the Y plane, which is an overestimate of the total texture memory.
    {.fFormat=TextureFormat::kYUV8_P2_420,
     .fChannels={{'y', 8, UNorm}, {'u', 8, UNorm}, {'v', 8, UNorm}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kYUV8_P3_420,
     .fChannels={{'y', 8, UNorm}, {'u', 8, UNorm}, {'v', 8, UNorm}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGB_888x_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kYUV10x6_P2_420,
     .fChannels={{'y', 10, UNorm}, {'x', 6, Pad}, {'u', 10, UNorm}, {'x', 6, Pad},
                 {'v', 10, UNorm}, {'x', 6, Pad}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGBA_10x6_SkColorType, Swizzle::RGBA(), std::nullopt}}},

    {.fFormat=TextureFormat::kExternal,
     // We don't really know this, but most Skia behavior defaults to assuming 8-bit color
     .fChannels={{'r', 8, UNorm}, {'g', 8, UNorm}, {'b', 8, UNorm}, {'a', 8, UNorm}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={{kRGBA_8888_SkColorType, Swizzle::RGBA(), std::nullopt},
                             {kRGB_888x_SkColorType,  Swizzle::RGB1(), std::nullopt}}},

    {.fFormat=TextureFormat::kS8,
     .fChannels={{'s', 8, Signed}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kD16,
     .fChannels={{'d', 16, UNorm}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kD32F,
     .fChannels={{'d', 32, Float}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kD24_S8,
     .fChannels={{'d', 24, UNorm}, {'s', 8, Signed}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},

    {.fFormat=TextureFormat::kD32F_S8,
     .fChannels={{'d', 32, Float}, {'s', 8, Signed}},
     .fXferSwizzle=std::nullopt,
     .fCompatibleColorTypes={}},
};

void test_format_transfers(skiatest::Reporter* r,
                           const FormatExpectation& textureFormat,
                           const ColorTypeExpectation& textureCT) {
    // When transferring to CPU->GPU, we want to apply the textureCT's write swizzle, but if that
    // is undefined because rendering is disabled, switch to RGB1. This is applicable for the
    // RGBx cases and for gray (alongside adjusting the texture channel to produce 'G').
    Swizzle writeSwizzle = textureCT.fWriteSwizzle.value_or(Swizzle::RGB1());
    skia_private::TArray<Channel> expectedTextureChannels = textureFormat.fChannels;
    // Adjust the R8 channel to be 'G' for gray-storing textures so that gen_pixel_data includes
    // any conversion to or from luminance.
    if (textureCT.fColorType == kGray_8_SkColorType) {
        SkASSERT(expectedTextureChannels.size() == 1 && expectedTextureChannels[0].fName == 'r');
        expectedTextureChannels[0].fName = 'G';
    }

    SkString gpuLabel = SkStringPrintf("GPU format %s as %s",
            TextureFormatName(textureFormat.fFormat),
            ToolUtils::colortype_name(textureCT.fColorType));
    // Transfering from srcCT into a GPU textureFormat interpreted as textureCT
    for (const ColorTypeChannels& src : kColorTypeChannels) {
        std::optional<TextureFormatXferFn> xferFn =
                TextureFormatXferFn::MakeCpuToGpu(src.fColorType,
                                                  textureFormat.fFormat,
                                                  textureCT.fReadSwizzle);
        REPORTER_ASSERT(r, textureFormat.fXferSwizzle.has_value() == xferFn.has_value());

        if (textureFormat.fXferSwizzle.has_value() && xferFn.has_value()) {
            PixelData cpuPixel = gen_pixel_data(src.fChannels);

            // The expected GPU value is formed by applying the source colortype's effective
            // swizzle (i.e. fill in missing channels), and the format's compatible colortype's
            // write swizzle to the channel definition of format.
            PixelData expectedGpuPixel = gen_pixel_data(
                        expectedTextureChannels,
                        Swizzle::Concat(src.fEffectiveSwizzle, writeSwizzle),
                        src.fChannels);
            PixelData actualGpuPixel = transfer_data(*xferFn, cpuPixel);

            const int tol = channel_tolerance(expectedTextureChannels, src.fChannels);
            if (!compare_pixels(textureFormat.fChannels, expectedGpuPixel, actualGpuPixel, tol)) {
                SkString ctLabel = SkStringPrintf("CPU colortype %s",
                                                  ToolUtils::colortype_name(src.fColorType));
                dump_pixel_comparison(ctLabel,
                                      src.fChannels,
                                      cpuPixel,
                                      gpuLabel,
                                      textureFormat.fChannels,
                                      expectedGpuPixel,
                                      actualGpuPixel);
                REPORTER_ASSERT(r, false,  "Pixel mismatch uploading from %s", ctLabel.c_str());
                STOP_ON_TRANSFER_FAILURE
            }
        }
    }

    // Transfering from a GPU textureFormat interpreted as textureCT into dstCT
    for (const ColorTypeChannels& dst : kColorTypeChannels) {
        std::optional<TextureFormatXferFn> xferFn =
                TextureFormatXferFn::MakeGpuToCpu(textureFormat.fFormat,
                                                  textureCT.fReadSwizzle,
                                                  dst.fColorType);
        REPORTER_ASSERT(r, textureFormat.fXferSwizzle.has_value() == xferFn.has_value());

        if (textureFormat.fXferSwizzle.has_value() && xferFn.has_value()) {
            PixelData gpuPixel = gen_pixel_data(expectedTextureChannels);

            // The expected CPU value is formed by applying the TextureFormat's implicit transfer
            // swizzle (i.e. fill in missing channels), its compatible colortype's read swizzle
            // to the channel definition of the dst color type.
            PixelData expectedCpuPixel = gen_pixel_data(
                    dst.fChannels,
                    Swizzle::Concat(*textureFormat.fXferSwizzle, textureCT.fReadSwizzle),
                    expectedTextureChannels);

            PixelData actualCpuPixel = transfer_data(*xferFn, gpuPixel);

            const int tol = channel_tolerance(dst.fChannels, expectedTextureChannels);
            if (!compare_pixels(dst.fChannels, expectedCpuPixel, actualCpuPixel, tol)) {
                SkString ctLabel = SkStringPrintf("CPU colortype %s",
                                                  ToolUtils::colortype_name(dst.fColorType));

                dump_pixel_comparison(gpuLabel,
                                      expectedTextureChannels,
                                      gpuPixel,
                                      ctLabel,
                                      dst.fChannels,
                                      expectedCpuPixel,
                                      actualCpuPixel);
                REPORTER_ASSERT(r, false,  "Pixel mismatch reading back to %s", ctLabel.c_str());
                STOP_ON_TRANSFER_FAILURE
            }
        }
    }
}

void run_texture_format_test(skiatest::Reporter* r, const Caps* caps, TextureFormat format) {
    bool foundExpectation = false;
    for (auto&& e : kExpectations) {
        if (e.fFormat != format) {
            continue;
        }

        // Should only find it once
        REPORTER_ASSERT(r, !foundExpectation, "Format expectation listed multiple times");
        foundExpectation = true;

        skiatest::ReporterContext scope(r, SkStringPrintf("Format %s", TextureFormatName(format)));

        // Found the expectation for the requested format. Check fixed properties first.
        REPORTER_ASSERT(r, e.fCompressionType == TextureFormatCompressionType(format));
        REPORTER_ASSERT(r, e.bytesPerBlock() == TextureFormatBytesPerBlock(format));
        REPORTER_ASSERT(r, e.channelMask() == TextureFormatChannelMask(format));
        REPORTER_ASSERT(r, e.hasDepthOrStencil() == TextureFormatIsDepthOrStencil(format));
        REPORTER_ASSERT(r, e.hasDepth() == TextureFormatHasDepth(format));
        REPORTER_ASSERT(r, e.hasStencil() == TextureFormatHasStencil(format));
        REPORTER_ASSERT(r, e.isMultiplanar() == TextureFormatIsMultiplanar(format));
        REPORTER_ASSERT(r, e.autoClamps() == TextureFormatAutoClamps(format));
        REPORTER_ASSERT(r, e.isFloatingPoint() == TextureFormatIsFloatingPoint(format));

        // Verify compatible color types
        auto [baseColorType, _] = TextureFormatColorTypeInfo(format);
        if (baseColorType == kUnknown_SkColorType) {
            REPORTER_ASSERT(r, e.fCompatibleColorTypes.empty());
        } else {
            // Should be the first listed compatible color type
            REPORTER_ASSERT(r, !e.fCompatibleColorTypes.empty());
            REPORTER_ASSERT(r, e.fCompatibleColorTypes[0].fColorType == baseColorType);
        }

        for (int c = 0; c <= kLastEnum_SkColorType; ++c) {
            SkColorType ct = static_cast<SkColorType>(c);

            skiatest::ReporterContext ctScope{
                    r, SkStringPrintf("color type %s\n", ToolUtils::colortype_name(ct))};

            bool foundColorExpectation = false;
            for (auto&& ec : e.fCompatibleColorTypes) {
                if (ec.fColorType == ct) {
                    // Expected to be compatible (and should only find it once)
                    REPORTER_ASSERT(r, !foundColorExpectation,
                                    "Color type listed multiple times: %s",
                                    ToolUtils::colortype_name(ec.fColorType));
                    foundColorExpectation = true;

                    // Check swizzles and transfers here, the rest of the color type checks happen
                    // outside the loop based on `foundColorExpectation`.
                    Swizzle actualReadSwizzle = ReadSwizzleForColorType(ct, format);
                    REPORTER_ASSERT(r, ec.fReadSwizzle == actualReadSwizzle,
                                    "actual %s vs. expected %s",
                                    actualReadSwizzle.asString().c_str(),
                                    ec.fReadSwizzle.asString().c_str());

                    auto actualWriteSwizzle = WriteSwizzleForColorType(ct, format);
                    if (ec.fWriteSwizzle.has_value()) {
                        REPORTER_ASSERT(r, actualWriteSwizzle.has_value());
                        REPORTER_ASSERT(r, ec.fWriteSwizzle == actualWriteSwizzle,
                                        "actual %s vs. expected %s",
                                        actualWriteSwizzle ? actualWriteSwizzle->asString().c_str()
                                                           : "null",
                                        ec.fWriteSwizzle->asString().c_str());
                    } else {
                        REPORTER_ASSERT(r, !actualWriteSwizzle.has_value());
                        // This is a proxy for "the format can represent CT, and there are some
                        // formats that can render CT, but this format does not render w/ CT".
                        TextureInfo renderableInfo = caps->getDefaultSampledTextureInfo(
                                ct, Mipmapped::kNo, Protected::kNo, Renderable::kYes);
                        REPORTER_ASSERT(r, format != TextureInfoPriv::ViewFormat(renderableInfo));
                    }

                    test_format_transfers(r, e, ec);
                }
            }

            // If we found an expectation, it should be detected as compatible (and false otherwise)
            const bool actualCompatible = AreColorTypeAndFormatCompatible(ct, format);
            REPORTER_ASSERT(r, foundColorExpectation == actualCompatible,
                            "actual (%d) vs expected (%d)",
                            actualCompatible, foundColorExpectation);
        }
    }

    // All formats should have expectations
    REPORTER_ASSERT(r, foundExpectation, "Missing expectation for %s", TextureFormatName(format));
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(TextureFormatTest, r, ctx, CtsEnforcement::kNextRelease) {
    for (int i = 0; i < kTextureFormatCount; ++i) {
        run_texture_format_test(r, ctx->priv().caps(), static_cast<TextureFormat>(i));
    }
}

} // namespace skgpu::graphite
