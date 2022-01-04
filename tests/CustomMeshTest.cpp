/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkCustomMesh.h"
#include "tests/Test.h"

using Attribute = SkCustomMeshSpecification::Attribute;
using Varying   = SkCustomMeshSpecification::Varying;

static const char* attr_type_str(const Attribute::Type type) {
    switch (type) {
        case Attribute::Type::kFloat:           return "float";
        case Attribute::Type::kFloat2:          return "float2";
        case Attribute::Type::kFloat3:          return "float3";
        case Attribute::Type::kFloat4:          return "float4";
        case Attribute::Type::kUByte4_unorm:    return "ubyte4_unorm";
    }
    SkUNREACHABLE;
}

static const char* var_type_str(const Varying::Type type) {
    switch (type) {
        case Varying::Type::kFloat:  return "float";
        case Varying::Type::kFloat2: return "float2";
        case Varying::Type::kFloat3: return "float3";
        case Varying::Type::kFloat4: return "float4";
        case Varying::Type::kHalf:   return "half";
        case Varying::Type::kHalf2:  return "half2";
        case Varying::Type::kHalf3:  return "half3";
        case Varying::Type::kHalf4:  return "half4";
    }
    SkUNREACHABLE;
}

static SkString make_description(SkSpan<const Attribute> attributes,
                                 size_t                  stride,
                                 SkSpan<const Varying>   varyings,
                                 const SkString&         vs,
                                 const SkString&         fs) {
    static constexpr size_t kMax = 10;
    SkString result;
    result.appendf("Attributes (count=%zu, stride=%zu):\n", attributes.size(), stride);
    for (size_t i = 0; i < std::min(kMax, attributes.size()); ++i) {
        const auto& a = attributes[i];
        result.appendf(" {%-10s, %3zu, \"%s\"}\n",
                       attr_type_str(a.type),
                       a.offset,
                       a.name.c_str());
    }
    if (kMax < attributes.size()) {
        result.append(" ...\n");
    }

    result.appendf("Varyings (count=%zu):\n", varyings.size());
    for (size_t i = 0; i < std::min(kMax, varyings.size()); ++i) {
        const auto& v = varyings[i];
        result.appendf(" {%5s, \"%s\"}\n", var_type_str(v.type), v.name.c_str());
    }
    if (kMax < varyings.size()) {
        result.append(" ...\n");
    }

    result.appendf("\n--VS--\n%s\n------\n", vs.c_str());
    result.appendf("\n--FS--\n%s\n------\n", fs.c_str());
    return result;
}

static bool check_for_failure(skiatest::Reporter*     r,
                              SkSpan<const Attribute> attributes,
                              size_t                  stride,
                              SkSpan<const Varying>   varyings,
                              const SkString&         vs,
                              const SkString&         fs) {
    auto [spec, error] = SkCustomMeshSpecification::Make(attributes, stride, varyings, vs, fs);
    SkString description;
    if (!spec) {
        return true;
    }
    ERRORF(r,
           "Expected to fail but succeeded:\n%s",
           make_description(attributes, stride, varyings, vs, fs).c_str());
    return false;
}

static bool check_for_success(skiatest::Reporter*     r,
                              SkSpan<const Attribute> attributes,
                              size_t                  stride,
                              SkSpan<const Varying>   varyings,
                              const SkString&         vs,
                              const SkString&         fs) {
    auto [spec, error] = SkCustomMeshSpecification::Make(attributes, stride, varyings, vs, fs);
    if (spec) {
        REPORTER_ASSERT(r, error.isEmpty());
        return true;
    }
    ERRORF(r,
           "Expected to succeed but failed:\n%sError:\n%s",
           make_description(attributes, stride, varyings, vs, fs).c_str(),
           error.c_str());
    return false;
}

// Simple valid strings to make specifications
static const SkString kValidVS
        {"float2 main(Attributes attrs, out Varyings v) { return float2(10); }"};
// There are multiple valid VS signatures.
static const SkString kValidFSes[] {
        SkString{"void main(Varyings varyings) {}"},
        SkString{"float2 main(Varyings varyings) { return float2(10); }"},
        SkString{"void main(Varyings varyings, out half4 color) { color = half4(.2); }"},
        SkString{R"(
            float2 main(Varyings varyings, out half4 color) {
                color = half4(.2);
                return float2(10);
            }
        )"},
};

// Simple valid attributes, stride, and varyings to make specifications
static const Attribute kValidAttrs[] = {
        {Attribute::Type::kFloat4, 0, SkString{"pos"}},
};
static constexpr size_t kValidStride = 4*4;
static const Varying kValidVaryings[] = {
        {Varying::Type::kFloat2, SkString{"uv"}},
};

static void test_good(skiatest::Reporter* r) {
    for (const auto& validFS : kValidFSes) {
        if (!check_for_success(r,
                               SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                               kValidStride,
                               SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                               kValidVS,
                               validFS)) {
            return;
        }
    }
}

static void test_bad_sig(skiatest::Reporter* r) {
    static constexpr const char* kVSBody = "{ return float2(10); }";

    static constexpr const char* kInvalidVSSigs[] {
            "float3 main(Attributes attrs, out Varyings v)",         // bad return
            "float2 main(inout Attributes attrs, out Varyings v)",   // inout Attributes
            "float2 main(Attributes attrs, inout Varyings v)",       // inout Varyings
            "float2 main(Attributes attrs)",                         // no Varyings
            "float2 main(out Varyings)",                             // no Attributes
            "float2 main(out Varyings, in Attributes)",              // wrong param order
            "float2 main(Attributes attrs, out Varyings v, float2)"  // extra arg
    };

    static constexpr const char* kNoColorFSBody = "{ return float2(10); }";

    static constexpr const char* kInvalidNoColorFSSigs[] {
            "half2 main(Varyings v)",               // bad return
            "float2 main(in Attributes v)",         // wrong param type
            "float2 main(out Varyings attrs)",      // out Varyings
            "float2 main()",                        // no args
            "float2 main(Attributes attrs, float)"  // extra arg
    };

    static constexpr const char* kColorFSBody = "{ color = half4(.2); return float2(10); }";

    static constexpr const char* kInvalidColorFSSigs[] {
            "half2 main(Varyings v, out half4 color)",               // bad return
            "float2 main(in Attributes v, out half4 color)",         // wrong first param type
            "float2 main(in Varyings v, out half3 color)",           // wrong second param type
            "float2 main(out Varyings attrs, out half4 color)",      // out Varyings
            "float2 main(Varyings attrs, half4 color)",              // in color
            "float2 main(Attributes attrs, out half4 color, float)"  // extra arg
    };

    for (const char* vsSig : kInvalidVSSigs) {
        SkString invalidVS;
        invalidVS.appendf("%s %s", vsSig, kVSBody);
        for (const auto& validFS : kValidFSes) {
            if (!check_for_failure(r,
                                   SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                                   kValidStride,
                                   SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                                   invalidVS,
                                   validFS)) {
                return;
            }
        }
    }

    for (const char* noColorFSSig : kInvalidNoColorFSSigs) {
        SkString invalidFS;
        invalidFS.appendf("%s %s", noColorFSSig, kNoColorFSBody);
        if (!check_for_failure(r,
                               SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                               kValidStride,
                               SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                               kValidVS,
                               invalidFS)) {
            return;
        }
    }

    for (const char* colorFSSig : kInvalidColorFSSigs) {
        SkString invalidFS;
        invalidFS.appendf("%s %s", colorFSSig, kColorFSBody);
        if (!check_for_failure(r,
                               SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                               kValidStride,
                               SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                               kValidVS,
                               invalidFS)) {
            return;
        }
    }
}

// We allow the optional out color from the FS to either be float4 or half4
static void test_float4_color(skiatest::Reporter* r) {
    static const SkString kFloat4FS {
        R"(
            float2 main(Varyings varyings, out float4 color) {
                color = float4(.2); return float2(10);
            }
        )"
    };
    check_for_success(r,
                      SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                      kValidStride,
                      SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                      kValidVS,
                      kFloat4FS);
}

// We don't allow value or child uniforms in custom meshes currently.
static void test_bad_globals(skiatest::Reporter* r) {
    static constexpr const char* kBadGlobals[] {
        "uniform float3 uni;"
        "uniform shader myshader;"
    };
    for (const auto& global : kBadGlobals) {
        SkString badVS = kValidVS;
        badVS.prepend(global);
        if (!check_for_failure(r,
                               SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                               kValidStride,
                               SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                               badVS,
                               kValidFSes[0])) {
            return;
        }
    }
    for (const auto& global : kBadGlobals) {
        SkString badFS = kValidFSes[0];
        badFS.prepend(global);
        if (!check_for_failure(r,
                               SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                               kValidStride,
                               SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                               kValidVS,
                               badFS)) {
            return;
        }
    }
}

static void test_no_main(skiatest::Reporter* r) {
    static const SkString kHelper{"float2 swiz(float2 x) { return z.yx; }"};

    // Empty VS
    if (!check_for_failure(r,
                           SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                           kValidStride,
                           SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                           SkString{},
                           kValidFSes[0])) {
        return;
    }

    // VS with helper function but no main
    if (!check_for_failure(r,
                           SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                           kValidStride,
                           SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                           kHelper,
                           kValidFSes[0])) {
        return;
    }

    // Empty FS
    if (!check_for_failure(r,
                           SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                           kValidStride,
                           SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                           kValidVS,
                           SkString{})) {
        return;
    }

    // VS with helper function but no main
    if (!check_for_failure(r,
                           SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                           kValidStride,
                           SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                           kValidVS,
                           kHelper)) {
        return;
    }
}

static void test_zero_attrs(skiatest::Reporter* r) {
    // We require at least one attribute
    check_for_failure(r,
                      SkSpan<Attribute>(),
                      kValidStride,
                      SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                      kValidVS,
                      kValidFSes[0]);
}

static void test_zero_varyings(skiatest::Reporter* r) {
    // Varyings are not required.
    check_for_success(r,
                      SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                      kValidStride,
                      SkSpan<Varying>(),
                      kValidVS,
                      kValidFSes[0]);
}

static void test_bad_strides(skiatest::Reporter* r) {
    // Zero stride
    if (!check_for_failure(r,
                           SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                           0,
                           SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                           kValidVS,
                           kValidFSes[0])) {
        return;
    }

    // Unaligned
    if (!check_for_failure(r,
                           SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                           kValidStride + 1,
                           SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                           kValidVS,
                           kValidFSes[0])) {
        return;
    }

    // Too large
    if (!check_for_failure(r,
                           SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                           1 << 20,
                           SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                           kValidVS,
                           kValidFSes[0])) {
        return;
    }
}

static void test_bad_offsets(skiatest::Reporter* r) {
    {  // offset isn't aligned
        static const Attribute kAttributes[] {
                {Attribute::Type::kFloat4,  1, SkString{"var"}},
        };
        if (!check_for_failure(r,
                               SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                               32,
                               SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                               kValidVS,
                               kValidFSes[0])) {
            return;
        }
    }
    {  // straddles stride boundary
        static const Attribute kAttributes[] {
                {Attribute::Type::kFloat4,   0, SkString{"var"}},
                {Attribute::Type::kFloat2,  16, SkString{"var"}},
        };
        if (!check_for_failure(r,
                               SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                               20,
                               SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                               kValidVS,
                               kValidFSes[0])) {
            return;
        }
    }
    {  // straddles stride boundary with attempt to overflow
        static const Attribute kAttributes[] {
                {Attribute::Type::kFloat, std::numeric_limits<size_t>::max() - 3, SkString{"var"}},
        };
        if (!check_for_failure(r,
                               SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                               4,
                               SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                               kValidVS,
                               kValidFSes[0])) {
            return;
        }
    }
}

static void test_too_many_attrs(skiatest::Reporter* r) {
    static constexpr size_t kN = 500;
    std::vector<Attribute> attrs;
    attrs.reserve(kN);
    for (size_t i = 0; i < kN; ++i) {
        attrs.push_back({Attribute::Type::kFloat4, 0, SkStringPrintf("attr%zu", i)});
    }
    check_for_failure(r,
                      SkMakeSpan(attrs),
                      4*4,
                      SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                      kValidVS,
                      kValidFSes[0]);
}

static void test_too_many_varyings(skiatest::Reporter* r) {
    static constexpr size_t kN = 500;
    std::vector<Varying> varyings;
    varyings.reserve(kN);
    for (size_t i = 0; i < kN; ++i) {
        varyings.push_back({Varying::Type::kFloat4, SkStringPrintf("varying%zu", i)});
    }
    check_for_failure(r,
                      SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                      kValidStride,
                      SkMakeSpan(varyings),
                      kValidVS,
                      kValidFSes[0]);
}

static void test_duplicate_attribute_names(skiatest::Reporter* r) {
    static const Attribute kAttributes[] {
            {Attribute::Type::kFloat4,  0, SkString{"var"}},
            {Attribute::Type::kFloat2, 16, SkString{"var"}}
    };
    check_for_failure(r,
                      SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                      24,
                      SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                      kValidVS,
                      kValidFSes[0]);
}

static void test_duplicate_varying_names(skiatest::Reporter* r) {
    static const Varying kVaryings[] {
        {Varying::Type::kFloat4, SkString{"var"}},
        {Varying::Type::kFloat3, SkString{"var"}}
    };
    check_for_failure(r,
                      SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                      kValidStride,
                      SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                      kValidVS,
                      kValidFSes[0]);
}

static constexpr const char* kSneakyName = "name; float3 sneaky";

static void test_sneaky_attribute_name(skiatest::Reporter* r) {
    static const Attribute kAttributes[] {
            {Attribute::Type::kFloat4, 0, SkString{kSneakyName}},
    };
    check_for_failure(r,
                      SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                      16,
                      SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                      kValidVS,
                      kValidFSes[0]);
}

static void test_sneaky_varying_name(skiatest::Reporter* r) {
    static const Varying kVaryings[] {
            {Varying::Type::kFloat4, SkString{kSneakyName}},
    };
    check_for_failure(r,
                      SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                      kValidStride,
                      SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                      kValidVS,
                      kValidFSes[0]);
}

static void test_empty_attribute_name(skiatest::Reporter* r) {
    static const Attribute kAttributes[] {
            {Attribute::Type::kFloat4, 0, SkString{}},
    };
    check_for_failure(r,
                      SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                      16,
                      SkMakeSpan(kValidVaryings, SK_ARRAY_COUNT(kValidVaryings)),
                      kValidVS,
                      kValidFSes[0]);
}

static void test_empty_varying_name(skiatest::Reporter* r) {
    static const Varying kVaryings[] {
            {Varying::Type::kFloat4, SkString{}},
    };
    check_for_failure(r,
                      SkMakeSpan(kValidAttrs, SK_ARRAY_COUNT(kValidAttrs)),
                      kValidStride,
                      SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(CustomMeshSpec, reporter) {
    struct X {};
    test_good(reporter);
    test_bad_sig(reporter);
    test_float4_color(reporter);
    test_bad_globals(reporter);
    test_no_main(reporter);
    test_zero_attrs(reporter);
    test_zero_varyings(reporter);
    test_bad_strides(reporter);
    test_bad_offsets(reporter);
    test_too_many_attrs(reporter);
    test_too_many_varyings(reporter);
    // skbug.com/12712
    if (0) {
        test_duplicate_attribute_names(reporter);
        test_duplicate_varying_names(reporter);
    }
    test_sneaky_attribute_name(reporter);
    test_sneaky_varying_name(reporter);
    test_empty_attribute_name(reporter);
    test_empty_varying_name(reporter);
}
