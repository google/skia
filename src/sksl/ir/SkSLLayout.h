/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_LAYOUT
#define SKSL_LAYOUT

#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLUtil.h"

namespace SkSL {

/**
 * Represents a layout block appearing before a variable declaration, as in:
 *
 * layout (location = 0) int x;
 */
struct Layout {
    enum Flag {
        kOriginUpperLeft_Flag            = 1 <<  0,
        kOverrideCoverage_Flag           = 1 <<  1,
        kPushConstant_Flag               = 1 <<  2,
        kBlendSupportAllEquations_Flag   = 1 <<  3,
        kBlendSupportMultiply_Flag       = 1 <<  4,
        kBlendSupportScreen_Flag         = 1 <<  5,
        kBlendSupportOverlay_Flag        = 1 <<  6,
        kBlendSupportDarken_Flag         = 1 <<  7,
        kBlendSupportLighten_Flag        = 1 <<  8,
        kBlendSupportColorDodge_Flag     = 1 <<  9,
        kBlendSupportColorBurn_Flag      = 1 << 10,
        kBlendSupportHardLight_Flag      = 1 << 11,
        kBlendSupportSoftLight_Flag      = 1 << 12,
        kBlendSupportDifference_Flag     = 1 << 13,
        kBlendSupportExclusion_Flag      = 1 << 14,
        kBlendSupportHSLHue_Flag         = 1 << 15,
        kBlendSupportHSLSaturation_Flag  = 1 << 16,
        kBlendSupportHSLColor_Flag       = 1 << 17,
        kBlendSupportHSLLuminosity_Flag  = 1 << 18,
        kTracked_Flag                    = 1 << 19
    };

    enum Primitive {
        kUnspecified_Primitive = -1,
        kPoints_Primitive,
        kLines_Primitive,
        kLineStrip_Primitive,
        kLinesAdjacency_Primitive,
        kTriangles_Primitive,
        kTriangleStrip_Primitive,
        kTrianglesAdjacency_Primitive
    };

    // These are used by images in GLSL. We only support a subset of what GL supports.
    enum class Format {
        kUnspecified = -1,
        kRGBA32F,
        kR32F,
        kRGBA16F,
        kR16F,
        kLUMINANCE16F,
        kRGBA8,
        kR8,
        kRGBA8I,
        kR8I,
        kRG16F,
    };

    // used by SkSL processors
    enum Key {
        // field is not a key
        kNo_Key,
        // field is a key
        kKey_Key,
        // key is 0 or 1 depending on whether the matrix is an identity matrix
        kIdentity_Key,
    };

    enum class CType {
        kDefault,
        kBool,
        kFloat,
        kFloat2,
        kFloat3,
        kFloat4,
        kInt32,
        kSkRect,
        kSkIRect,
        kSkPMColor4f,
        kSkPMColor,
        kSkV4,
        kSkPoint,
        kSkIPoint,
        kSkMatrix,
        kSkM44,
        kGrSurfaceProxyView,
        kGrFragmentProcessor,
    };

    static const char* FormatToStr(Format format) {
        switch (format) {
            case Format::kUnspecified:  return "";
            case Format::kRGBA32F:      return "rgba32f";
            case Format::kR32F:         return "r32f";
            case Format::kRGBA16F:      return "rgba16f";
            case Format::kR16F:         return "r16f";
            case Format::kLUMINANCE16F: return "lum16f";
            case Format::kRGBA8:        return "rgba8";
            case Format::kR8:           return "r8";
            case Format::kRGBA8I:       return "rgba8i";
            case Format::kR8I:          return "r8i";
            case Format::kRG16F:        return "rg16f";
        }
        ABORT("Unexpected format");
    }

    static bool ReadFormat(String str, Format* format) {
        if (str == "rgba32f") {
            *format = Format::kRGBA32F;
            return true;
        } else if (str == "r32f") {
            *format = Format::kR32F;
            return true;
        } else if (str == "rgba16f") {
            *format = Format::kRGBA16F;
            return true;
        } else if (str == "r16f") {
            *format = Format::kR16F;
            return true;
        } else if (str == "lum16f") {
            *format = Format::kLUMINANCE16F;
            return true;
        } else if (str == "rgba8") {
            *format = Format::kRGBA8;
            return true;
        } else if (str == "r8") {
            *format = Format::kR8;
            return true;
        } else if (str == "rgba8i") {
            *format = Format::kRGBA8I;
            return true;
        } else if (str == "r8i") {
            *format = Format::kR8I;
            return true;
        } else if (str == "rg16f") {
            *format = Format::kRG16F;
            return true;
        }
        return false;
    }

    static const char* CTypeToStr(CType ctype) {
        switch (ctype) {
            case CType::kDefault:
                return nullptr;
            case CType::kFloat:
                return "float";
            case CType::kInt32:
                return "int32_t";
            case CType::kSkRect:
                return "SkRect";
            case CType::kSkIRect:
                return "SkIRect";
            case CType::kSkPMColor4f:
                return "SkPMColor4f";
            case CType::kSkPMColor:
                return "SkPMColor";
            case CType::kSkV4:
                return "SkV4";
            case CType::kSkPoint:
                return "SkPoint";
            case CType::kSkIPoint:
                return "SkIPoint";
            case CType::kSkMatrix:
                return "SkMatrix";
            case CType::kSkM44:
                return "SkM44";
            case CType::kGrSurfaceProxyView:
                return "GrSurfaceProxyView";
            case CType::kGrFragmentProcessor:
                return "std::unique_ptr<GrFragmentProcessor>";
            default:
                SkASSERT(false);
                return nullptr;
        }
    }

    Layout(int flags, int location, int offset, int binding, int index, int set, int builtin,
           int inputAttachmentIndex, Format format, Primitive primitive, int maxVertices,
           int invocations, StringFragment when, Key key, CType ctype)
    : fFlags(flags)
    , fLocation(location)
    , fOffset(offset)
    , fBinding(binding)
    , fIndex(index)
    , fSet(set)
    , fBuiltin(builtin)
    , fInputAttachmentIndex(inputAttachmentIndex)
    , fFormat(format)
    , fPrimitive(primitive)
    , fMaxVertices(maxVertices)
    , fInvocations(invocations)
    , fWhen(when)
    , fKey(key)
    , fCType(ctype) {}

    Layout()
    : fFlags(0)
    , fLocation(-1)
    , fOffset(-1)
    , fBinding(-1)
    , fIndex(-1)
    , fSet(-1)
    , fBuiltin(-1)
    , fInputAttachmentIndex(-1)
    , fFormat(Format::kUnspecified)
    , fPrimitive(kUnspecified_Primitive)
    , fMaxVertices(-1)
    , fInvocations(-1)
    , fKey(kNo_Key)
    , fCType(CType::kDefault) {}

    String description() const {
        String result;
        String separator;
        if (fLocation >= 0) {
            result += separator + "location = " + to_string(fLocation);
            separator = ", ";
        }
        if (fOffset >= 0) {
            result += separator + "offset = " + to_string(fOffset);
            separator = ", ";
        }
        if (fBinding >= 0) {
            result += separator + "binding = " + to_string(fBinding);
            separator = ", ";
        }
        if (fIndex >= 0) {
            result += separator + "index = " + to_string(fIndex);
            separator = ", ";
        }
        if (fSet >= 0) {
            result += separator + "set = " + to_string(fSet);
            separator = ", ";
        }
        if (fBuiltin >= 0) {
            result += separator + "builtin = " + to_string(fBuiltin);
            separator = ", ";
        }
        if (fInputAttachmentIndex >= 0) {
            result += separator + "input_attachment_index = " + to_string(fInputAttachmentIndex);
            separator = ", ";
        }
        if (Format::kUnspecified != fFormat) {
            result += separator + FormatToStr(fFormat);
            separator = ", ";
        }
        if (fFlags & kOriginUpperLeft_Flag) {
            result += separator + "origin_upper_left";
            separator = ", ";
        }
        if (fFlags & kOverrideCoverage_Flag) {
            result += separator + "override_coverage";
            separator = ", ";
        }
        if (fFlags & kBlendSupportAllEquations_Flag) {
            result += separator + "blend_support_all_equations";
            separator = ", ";
        }
        if (fFlags & kBlendSupportMultiply_Flag) {
            result += separator + "blend_support_multiply";
            separator = ", ";
        }
        if (fFlags & kBlendSupportScreen_Flag) {
            result += separator + "blend_support_screen";
            separator = ", ";
        }
        if (fFlags & kBlendSupportOverlay_Flag) {
            result += separator + "blend_support_overlay";
            separator = ", ";
        }
        if (fFlags & kBlendSupportDarken_Flag) {
            result += separator + "blend_support_darken";
            separator = ", ";
        }
        if (fFlags & kBlendSupportLighten_Flag) {
            result += separator + "blend_support_lighten";
            separator = ", ";
        }
        if (fFlags & kBlendSupportColorDodge_Flag) {
            result += separator + "blend_support_colordodge";
            separator = ", ";
        }
        if (fFlags & kBlendSupportColorBurn_Flag) {
            result += separator + "blend_support_colorburn";
            separator = ", ";
        }
        if (fFlags & kBlendSupportHardLight_Flag) {
            result += separator + "blend_support_hardlight";
            separator = ", ";
        }
        if (fFlags & kBlendSupportSoftLight_Flag) {
            result += separator + "blend_support_softlight";
            separator = ", ";
        }
        if (fFlags & kBlendSupportDifference_Flag) {
            result += separator + "blend_support_difference";
            separator = ", ";
        }
        if (fFlags & kBlendSupportExclusion_Flag) {
            result += separator + "blend_support_exclusion";
            separator = ", ";
        }
        if (fFlags & kBlendSupportHSLHue_Flag) {
            result += separator + "blend_support_hsl_hue";
            separator = ", ";
        }
        if (fFlags & kBlendSupportHSLSaturation_Flag) {
            result += separator + "blend_support_hsl_saturation";
            separator = ", ";
        }
        if (fFlags & kBlendSupportHSLColor_Flag) {
            result += separator + "blend_support_hsl_color";
            separator = ", ";
        }
        if (fFlags & kBlendSupportHSLLuminosity_Flag) {
            result += separator + "blend_support_hsl_luminosity";
            separator = ", ";
        }
        if (fFlags & kPushConstant_Flag) {
            result += separator + "push_constant";
            separator = ", ";
        }
        if (fFlags & kTracked_Flag) {
            result += separator + "tracked";
            separator = ", ";
        }
        switch (fPrimitive) {
            case kPoints_Primitive:
                result += separator + "points";
                separator = ", ";
                break;
            case kLines_Primitive:
                result += separator + "lines";
                separator = ", ";
                break;
            case kLineStrip_Primitive:
                result += separator + "line_strip";
                separator = ", ";
                break;
            case kLinesAdjacency_Primitive:
                result += separator + "lines_adjacency";
                separator = ", ";
                break;
            case kTriangles_Primitive:
                result += separator + "triangles";
                separator = ", ";
                break;
            case kTriangleStrip_Primitive:
                result += separator + "triangle_strip";
                separator = ", ";
                break;
            case kTrianglesAdjacency_Primitive:
                result += separator + "triangles_adjacency";
                separator = ", ";
                break;
            case kUnspecified_Primitive:
                break;
        }
        if (fMaxVertices >= 0) {
            result += separator + "max_vertices = " + to_string(fMaxVertices);
            separator = ", ";
        }
        if (fInvocations >= 0) {
            result += separator + "invocations = " + to_string(fInvocations);
            separator = ", ";
        }
        if (fWhen.fLength) {
            result += separator + "when = " + fWhen;
            separator = ", ";
        }
        if (result.size() > 0) {
            result = "layout (" + result + ")";
        }
        if (fKey) {
            result += "/* key */";
        }
        return result;
    }

    bool operator==(const Layout& other) const {
        return fFlags                == other.fFlags &&
               fLocation             == other.fLocation &&
               fOffset               == other.fOffset &&
               fBinding              == other.fBinding &&
               fIndex                == other.fIndex &&
               fSet                  == other.fSet &&
               fBuiltin              == other.fBuiltin &&
               fInputAttachmentIndex == other.fInputAttachmentIndex &&
               fFormat               == other.fFormat &&
               fPrimitive            == other.fPrimitive &&
               fMaxVertices          == other.fMaxVertices &&
               fInvocations          == other.fInvocations;
    }

    bool operator!=(const Layout& other) const {
        return !(*this == other);
    }

    int fFlags;
    int fLocation;
    int fOffset;
    int fBinding;
    int fIndex;
    int fSet;
    // builtin comes from SPIR-V and identifies which particular builtin value this object
    // represents.
    int fBuiltin;
    // input_attachment_index comes from Vulkan/SPIR-V to connect a shader variable to the a
    // corresponding attachment on the subpass in which the shader is being used.
    int fInputAttachmentIndex;
    Format fFormat;
    Primitive fPrimitive;
    int fMaxVertices;
    int fInvocations;
    StringFragment fWhen;
    Key fKey;
    CType fCType;
};

} // namespace

#endif
