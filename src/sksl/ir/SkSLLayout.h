/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_LAYOUT
#define SKSL_LAYOUT

#include "src/base/SkEnumBitMask.h"

#include <string>

namespace SkSL {

class Context;
class Position;

enum class LayoutFlag : int {
    kNone                       = 0,
    kAll                        = ~0,

    kOriginUpperLeft            = 1 <<  0,
    kPushConstant               = 1 <<  1,
    kBlendSupportAllEquations   = 1 <<  2,
    kColor                      = 1 <<  3,

    // These flags indicate if the qualifier appeared, regardless of the accompanying value.
    kLocation                   = 1 <<  4,
    kOffset                     = 1 <<  5,
    kBinding                    = 1 <<  6,
    kTexture                    = 1 <<  7,
    kSampler                    = 1 <<  8,
    kIndex                      = 1 <<  9,
    kSet                        = 1 << 10,
    kBuiltin                    = 1 << 11,
    kInputAttachmentIndex       = 1 << 12,

    // These flags indicate the backend type; only one at most can be set.
    kVulkan                     = 1 << 13,
    kMetal                      = 1 << 14,
    kWebGPU                     = 1 << 15,
    kDirect3D                   = 1 << 16,

    kAllBackends                = kVulkan | kMetal | kWebGPU | kDirect3D,

    // These flags indicate the pixel format; only one at most can be set.
    // (https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)#Image_formats)
    kRGBA8                      = 1 << 17,
    kRGBA32F                    = 1 << 18,
    kR32F                       = 1 << 19,

    kAllPixelFormats            = kRGBA8 | kRGBA32F | kR32F,

    // The local invocation size of a compute program.
    kLocalSizeX                 = 1 << 20,
    kLocalSizeY                 = 1 << 21,
    kLocalSizeZ                 = 1 << 22,
};

}  // namespace SkSL

SK_MAKE_BITMASK_OPS(SkSL::LayoutFlag);

namespace SkSL {

using LayoutFlags = SkEnumBitMask<SkSL::LayoutFlag>;

/**
 * Represents a layout block appearing before a variable declaration, as in:
 *
 * layout (location = 0) int x;
 */
struct Layout {
    Layout(LayoutFlags flags, int location, int offset, int binding, int index, int set,
           int builtin, int inputAttachmentIndex)
            : fFlags(flags)
            , fLocation(location)
            , fOffset(offset)
            , fBinding(binding)
            , fIndex(index)
            , fSet(set)
            , fBuiltin(builtin)
            , fInputAttachmentIndex(inputAttachmentIndex) {}

    constexpr Layout() = default;

    static Layout builtin(int builtin) {
        Layout result;
        result.fBuiltin = builtin;
        return result;
    }

    std::string description() const;
    std::string paddedDescription() const;

    /**
     * Verifies that only permitted layout flags are included. Reports errors and returns false in
     * the event of a violation.
     */
    bool checkPermittedLayout(const Context& context,
                              Position pos,
                              LayoutFlags permittedLayoutFlags) const;

    bool operator==(const Layout& other) const;

    bool operator!=(const Layout& other) const {
        return !(*this == other);
    }

    LayoutFlags fFlags = LayoutFlag::kNone;
    int fLocation = -1;
    int fOffset = -1;
    int fBinding = -1;
    int fTexture = -1;
    int fSampler = -1;
    int fIndex = -1;
    int fSet = -1;
    // builtin comes from SPIR-V and identifies which particular builtin value this object
    // represents.
    int fBuiltin = -1;
    // input_attachment_index comes from Vulkan/SPIR-V to connect a shader variable to the a
    // corresponding attachment on the subpass in which the shader is being used.
    int fInputAttachmentIndex = -1;

    // The local invocation size dimensions of a compute program.
    int fLocalSizeX = -1;
    int fLocalSizeY = -1;
    int fLocalSizeZ = -1;
};

}  // namespace SkSL

#endif
