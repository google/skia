/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_LAYOUT
#define SKSL_LAYOUT

#include <string>

namespace SkSL {

/**
 * Represents a layout block appearing before a variable declaration, as in:
 *
 * layout (location = 0) int x;
 */
struct Layout {
    enum Flag {
        kOriginUpperLeft_Flag            = 1 <<  0,
        kPushConstant_Flag               = 1 <<  1,
        kBlendSupportAllEquations_Flag   = 1 <<  2,
        kColor_Flag                      = 1 <<  3,

        // These flags indicate if the qualifier appeared, regardless of the accompanying value.
        kLocation_Flag                   = 1 <<  4,
        kOffset_Flag                     = 1 <<  5,
        kBinding_Flag                    = 1 <<  6,
        kTexture_Flag                    = 1 <<  7,
        kSampler_Flag                    = 1 <<  8,
        kIndex_Flag                      = 1 <<  9,
        kSet_Flag                        = 1 << 10,
        kBuiltin_Flag                    = 1 << 11,
        kInputAttachmentIndex_Flag       = 1 << 12,

        // These flags indicate the backend type; only one at most can be set.
        kSPIRV_Flag                      = 1 << 13,
        kMetal_Flag                      = 1 << 14,
        kGL_Flag                         = 1 << 15,
        kWGSL_Flag                       = 1 << 16,
    };

    static constexpr int kAllBackendFlagsMask =
            Layout::kSPIRV_Flag | Layout::kMetal_Flag | Layout::kGL_Flag | Layout::kWGSL_Flag;

    Layout(int flags, int location, int offset, int binding, int index, int set, int builtin,
           int inputAttachmentIndex)
    : fFlags(flags)
    , fLocation(location)
    , fOffset(offset)
    , fBinding(binding)
    , fIndex(index)
    , fSet(set)
    , fBuiltin(builtin)
    , fInputAttachmentIndex(inputAttachmentIndex) {}

    Layout() = default;

    static Layout builtin(int builtin) {
        Layout result;
        result.fBuiltin = builtin;
        return result;
    }

    std::string description() const;

    bool operator==(const Layout& other) const;

    bool operator!=(const Layout& other) const {
        return !(*this == other);
    }

    int fFlags = 0;
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
};

}  // namespace SkSL

#endif
