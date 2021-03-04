/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_LAYOUT
#define SKSL_LAYOUT

#include "include/private/SkSLString.h"

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
        kEarlyFragmentTests_Flag         = 1 <<  2,
        kPushConstant_Flag               = 1 <<  3,
        kBlendSupportAllEquations_Flag   = 1 <<  4,
        kTracked_Flag                    = 1 <<  5,
        kSRGBUnpremul_Flag               = 1 <<  6,
        kKey_Flag                        = 1 <<  7,

        // These flags indicate if the qualifier appeared, regardless of the accompanying value.
        kLocation_Flag                   = 1 <<  8,
        kOffset_Flag                     = 1 <<  9,
        kBinding_Flag                    = 1 << 10,
        kIndex_Flag                      = 1 << 11,
        kSet_Flag                        = 1 << 12,
        kBuiltin_Flag                    = 1 << 13,
        kInputAttachmentIndex_Flag       = 1 << 14,
        kPrimitive_Flag                  = 1 << 15,
        kMaxVertices_Flag                = 1 << 16,
        kInvocations_Flag                = 1 << 17,
        kMarker_Flag                     = 1 << 18,
        kWhen_Flag                       = 1 << 19,
        kCType_Flag                      = 1 << 20,
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
           int inputAttachmentIndex, Primitive primitive, int maxVertices, int invocations,
           StringFragment marker, StringFragment when, CType ctype)
    : fFlags(flags)
    , fLocation(location)
    , fOffset(offset)
    , fBinding(binding)
    , fIndex(index)
    , fSet(set)
    , fBuiltin(builtin)
    , fInputAttachmentIndex(inputAttachmentIndex)
    , fPrimitive(primitive)
    , fMaxVertices(maxVertices)
    , fInvocations(invocations)
    , fMarker(marker)
    , fWhen(when)
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
    , fPrimitive(kUnspecified_Primitive)
    , fMaxVertices(-1)
    , fInvocations(-1)
    , fCType(CType::kDefault) {}

    static Layout builtin(int builtin) {
        Layout result;
        result.fBuiltin = builtin;
        return result;
    }

    String description() const {
        String result;
        auto separator = [firstSeparator = true]() mutable -> String {
            if (firstSeparator) {
                firstSeparator = false;
                return "";
            } else {
                return ", ";
            }};
        if (fLocation >= 0) {
            result += separator() + "location = " + to_string(fLocation);
        }
        if (fOffset >= 0) {
            result += separator() + "offset = " + to_string(fOffset);
        }
        if (fBinding >= 0) {
            result += separator() + "binding = " + to_string(fBinding);
        }
        if (fIndex >= 0) {
            result += separator() + "index = " + to_string(fIndex);
        }
        if (fSet >= 0) {
            result += separator() + "set = " + to_string(fSet);
        }
        if (fBuiltin >= 0) {
            result += separator() + "builtin = " + to_string(fBuiltin);
        }
        if (fInputAttachmentIndex >= 0) {
            result += separator() + "input_attachment_index = " + to_string(fInputAttachmentIndex);
        }
        if (fFlags & kOriginUpperLeft_Flag) {
            result += separator() + "origin_upper_left";
        }
        if (fFlags & kOverrideCoverage_Flag) {
            result += separator() + "override_coverage";
        }
        if (fFlags & kEarlyFragmentTests_Flag) {
            result += separator() + "early_fragment_tests";
        }
        if (fFlags & kBlendSupportAllEquations_Flag) {
            result += separator() + "blend_support_all_equations";
        }
        if (fFlags & kPushConstant_Flag) {
            result += separator() + "push_constant";
        }
        if (fFlags & kTracked_Flag) {
            result += separator() + "tracked";
        }
        if (fFlags & kSRGBUnpremul_Flag) {
            result += separator() + "srgb_unpremul";
        }
        switch (fPrimitive) {
            case kPoints_Primitive:
                result += separator() + "points";
                break;
            case kLines_Primitive:
                result += separator() + "lines";
                break;
            case kLineStrip_Primitive:
                result += separator() + "line_strip";
                break;
            case kLinesAdjacency_Primitive:
                result += separator() + "lines_adjacency";
                break;
            case kTriangles_Primitive:
                result += separator() + "triangles";
                break;
            case kTriangleStrip_Primitive:
                result += separator() + "triangle_strip";
                break;
            case kTrianglesAdjacency_Primitive:
                result += separator() + "triangles_adjacency";
                break;
            case kUnspecified_Primitive:
                break;
        }
        if (fMaxVertices >= 0) {
            result += separator() + "max_vertices = " + to_string(fMaxVertices);
        }
        if (fInvocations >= 0) {
            result += separator() + "invocations = " + to_string(fInvocations);
        }
        if (fMarker.fLength) {
            result += separator() + "marker = " + fMarker;
        }
        if (fWhen.fLength) {
            result += separator() + "when = " + fWhen;
        }
        if (result.size() > 0) {
            result = "layout (" + result + ")";
        }
        if (fFlags & kKey_Flag) {
            result += "/* key */ const";
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
               fPrimitive            == other.fPrimitive &&
               fMaxVertices          == other.fMaxVertices &&
               fInvocations          == other.fInvocations &&
               fMarker               == other.fMarker &&
               fWhen                 == other.fWhen &&
               fCType                == other.fCType;
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
    Primitive fPrimitive;
    int fMaxVertices;
    int fInvocations;
    // marker refers to matrices tagged on the SkCanvas with markCTM
    StringFragment fMarker;
    StringFragment fWhen;
    CType fCType;
};

}  // namespace SkSL

#endif
