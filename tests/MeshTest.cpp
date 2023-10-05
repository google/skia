/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/base/SkZip.h"
#include "src/core/SkMeshPriv.h"
#include "tests/Test.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

using Attribute = SkMeshSpecification::Attribute;
using Varying   = SkMeshSpecification::Varying;

static const char* attr_type_str(const Attribute::Type type) {
    switch (type) {
        case Attribute::Type::kFloat:        return "float";
        case Attribute::Type::kFloat2:       return "float2";
        case Attribute::Type::kFloat3:       return "float3";
        case Attribute::Type::kFloat4:       return "float4";
        case Attribute::Type::kUByte4_unorm: return "ubyte4_unorm";
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
        result.appendf(" {%-10s, %3zu, \"%s\"}\n", attr_type_str(a.type), a.offset, a.name.c_str());
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

static bool check_for_failure(skiatest::Reporter*     reporter,
                              SkSpan<const Attribute> attributes,
                              size_t                  stride,
                              SkSpan<const Varying>   varyings,
                              const SkString&         vs,
                              const SkString&         fs,
                              const char*             expectedErrorSubstring = nullptr) {
    auto [spec, error] = SkMeshSpecification::Make(attributes, stride, varyings, vs, fs);
    if (spec) {
        ERRORF(reporter,
               "Expected to fail but succeeded:\n%s",
               make_description(attributes, stride, varyings, vs, fs).c_str());
        return false;
    }
    if (expectedErrorSubstring && !error.contains(expectedErrorSubstring)) {
        ERRORF(reporter,
               "    Expected: %s\n"
               "Actual error: %s\n",
               expectedErrorSubstring, error.c_str());
        return false;
    }
    return true;
}

static bool check_for_success(skiatest::Reporter*         reporter,
                              SkSpan<const Attribute>     attributes,
                              size_t                      stride,
                              SkSpan<const Varying>       varyings,
                              const SkString&             vs,
                              const SkString&             fs,
                              sk_sp<SkMeshSpecification>* spec = nullptr) {
    auto [s, error] = SkMeshSpecification::Make(attributes, stride, varyings, vs, fs);
    if (s) {
        REPORTER_ASSERT(reporter, error.isEmpty());
        if (spec) {
            *spec = std::move(s);
        }
        return true;
    }
    ERRORF(reporter,
           "Expected to succeed but failed:\n%sError:\n%s",
           make_description(attributes, stride, varyings, vs, fs).c_str(),
           error.c_str());
    return false;
}

// Simple valid strings to make specifications
static const SkString kValidVS {R"(
Varyings main(const Attributes attrs) {
    Varyings v;
    return v;
})"};

// There are multiple valid VS signatures.
static const SkString kValidFSes[]{
        SkString{"float2 main(const Varyings varyings) { return float2(10); }"},
        SkString{R"(
            float2 main(const Varyings varyings, out half4 color) {
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

DEF_TEST(MeshSpec_Valid, reporter) {
    for (const auto& validFS : kValidFSes) {
        if (!check_for_success(reporter,
                               kValidAttrs,
                               kValidStride,
                               kValidVaryings,
                               kValidVS,
                               validFS)) {
            return;
        }
    }
}

DEF_TEST(MeshSpec_InvalidSignature, reporter) {
    static constexpr const char* kVSBody = "{ return float2(10); }";

    static constexpr const char* kInvalidVSSigs[] {
            "float3   main(const Attributes attrs)",   // bad return
            "Varyings main(Attributes attrs)",         // non-const Attributes
            "Varyings main(out Attributes attrs)",     // out Varyings
            "Varyings main()",                         // no Attributes
            "Varyings main(const Varyings v, float2)"  // extra arg
    };

    static constexpr const char* kNoColorFSBody = "{ return float2(10); }";

    static constexpr const char* kInvalidNoColorFSSigs[] {
            "half2  main(const Varyings v)",      // bad return
            "float2 main(const Attributes v)",    // wrong param type
            "float2 main(inout Varyings attrs)",  // inout Varyings
            "float2 main(Varyings v)",            // non-const Varyings
            "float2 main()",                      // no args
            "float2 main(const Varyings, float)"  // extra arg
    };

    static constexpr const char* kColorFSBody = "{ color = half4(.2); return float2(10); }";

    static constexpr const char* kInvalidColorFSSigs[] {
            "half2  main(const Varyings v, out half4 color)",        // bad return
            "float2 main(const Attributes v, out half4 color)",      // wrong first param type
            "float2 main(const Varyings v, out half3 color)",        // wrong second param type
            "float2 main(out   Varyings v, out half4 color)",        // out Varyings
            "float2 main(const Varyings v, half4 color)",            // in color
            "float2 main(const Varyings v, out half4 color, float)"  // extra arg
    };

    for (const char* vsSig : kInvalidVSSigs) {
        SkString invalidVS;
        invalidVS.appendf("%s %s", vsSig, kVSBody);
        for (const auto& validFS : kValidFSes) {
            if (!check_for_failure(reporter,
                                   kValidAttrs,
                                   kValidStride,
                                   kValidVaryings,
                                   invalidVS,
                                   validFS)) {
                return;
            }
        }
    }

    for (const char* noColorFSSig : kInvalidNoColorFSSigs) {
        SkString invalidFS;
        invalidFS.appendf("%s %s", noColorFSSig, kNoColorFSBody);
        if (!check_for_failure(reporter,
                               kValidAttrs,
                               kValidStride,
                               kValidVaryings,
                               kValidVS,
                               invalidFS)) {
            return;
        }
    }

    for (const char* colorFSSig : kInvalidColorFSSigs) {
        SkString invalidFS;
        invalidFS.appendf("%s %s", colorFSSig, kColorFSBody);
        if (!check_for_failure(reporter,
                               kValidAttrs,
                               kValidStride,
                               kValidVaryings,
                               kValidVS,
                               invalidFS)) {
            return;
        }
    }
}

// We allow the optional out color from the FS to either be float4 or half4
DEF_TEST(MeshSpec_Float4Color, reporter) {
    static const SkString kFloat4FS {
        R"(
            float2 main(const Varyings varyings, out float4 color) {
                color = float4(.2); return float2(10);
            }
        )"
    };
    check_for_success(reporter,
                      kValidAttrs,
                      kValidStride,
                      kValidVaryings,
                      kValidVS,
                      kFloat4FS);
}

DEF_TEST(MeshSpec_DisallowsChildEffectInVertex, reporter) {
    static constexpr const char* kChildEffects[] {
        "uniform shader myshader;",
        "uniform colorFilter mycolorfilter;",
        "uniform blender myblender;"
    };

    for (const auto& global : kChildEffects) {
        SkString vsWithChild{global};
        vsWithChild.append(kValidVS);

        SkString fsWithChild{global};
        fsWithChild.append(kValidFSes[0]);

        if (!check_for_failure(reporter,
                               kValidAttrs,
                               kValidStride,
                               kValidVaryings,
                               vsWithChild,
                               kValidFSes[0],
                               "effects are not permitted in mesh vertex shaders")) {
            return;
        }

        if (!check_for_failure(reporter,
                               kValidAttrs,
                               kValidStride,
                               kValidVaryings,
                               vsWithChild,
                               fsWithChild,
                               "effects are not permitted in mesh vertex shaders")) {
            return;
        }
    }
}

DEF_TEST(MeshSpec_AllowsChildEffectInFragment, reporter) {
    static constexpr const char* kChildEffects[] {
        "uniform shader myshader;",
        "uniform colorFilter mycolorfilter; uniform shader myshader;",
        "uniform shader myshader; uniform blender myblender; uniform colorFilter mycolorfilter;"
    };

    for (const auto& global : kChildEffects) {
        SkString fsWithChild{global};
        fsWithChild.append(kValidFSes[0]);

        if (!check_for_success(reporter,
                               kValidAttrs,
                               kValidStride,
                               kValidVaryings,
                               kValidVS,
                               fsWithChild)) {
            return;
        }
    }
}

DEF_TEST(MeshSpec_FindChild, reporter) {
    SkString fsWithChild{"uniform shader myshader;"
                         "uniform blender myblender;"
                         "uniform colorFilter mycolorfilter;"};
    fsWithChild.append(kValidFSes[0]);

    sk_sp<SkMeshSpecification> meshSpec;
    if (!check_for_success(reporter,
                           kValidAttrs,
                           kValidStride,
                           kValidVaryings,
                           kValidVS,
                           fsWithChild,
                           &meshSpec)) {
        return;
    }

    REPORTER_ASSERT(reporter, meshSpec->findChild("myshader")->index == 0);
    REPORTER_ASSERT(reporter, meshSpec->findChild("myblender")->index == 1);
    REPORTER_ASSERT(reporter, meshSpec->findChild("mycolorfilter")->index == 2);
    REPORTER_ASSERT(reporter, !meshSpec->findChild("missing"));
}

DEF_TEST(Mesh_ChildEffectsMatchSpec, reporter) {
    auto test = [&](const char* prefix,
                    SkSpan<SkRuntimeEffect::ChildPtr> children,
                    const char* expectedError = nullptr) {
        SkString fsWithChild{prefix};
        fsWithChild.append(kValidFSes[0]);

        sk_sp<SkMeshSpecification> meshSpec;
        if (!check_for_success(reporter,
                               kValidAttrs,
                               kValidStride,
                               kValidVaryings,
                               kValidVS,
                               fsWithChild,
                               &meshSpec)) {
            return;
        }

        constexpr float kVertexCount = 4;
        sk_sp<SkMesh::VertexBuffer> vertexBuffer =
                SkMeshes::MakeVertexBuffer(nullptr, kValidStride * kVertexCount);
        SkMesh::Result result = SkMesh::Make(meshSpec,
                                             SkMesh::Mode::kTriangleStrip,
                                             vertexBuffer,
                                             kVertexCount,
                                             /*vertexOffset=*/0,
                                             /*uniforms=*/nullptr,
                                             children,
                                             SkRect::MakeEmpty());

        if (expectedError) {
            REPORTER_ASSERT(reporter, !result.mesh.isValid());
            REPORTER_ASSERT(reporter,
                            result.error.contains(expectedError),
                            "Expected: '%s'\n"
                            "  Actual: '%s'\n", expectedError, result.error.c_str());
        } else {
            REPORTER_ASSERT(reporter, result.mesh.isValid());
            REPORTER_ASSERT(reporter,
                            result.error.isEmpty(),
                            "Expected: no errors\n"
                            "  Actual: '%s'\n", result.error.c_str());
        }
    };

    SkRuntimeEffect::ChildPtr childShader[]  = {SkShaders::Color(SK_ColorBLACK)};
    SkRuntimeEffect::ChildPtr childFilter[]  = {SkColorFilters::LinearToSRGBGamma()};
    SkRuntimeEffect::ChildPtr childBlender[] = {SkBlender::Mode(SkBlendMode::kSrcOver)};
    SkRuntimeEffect::ChildPtr childNull[1]   = {};

    // These are expected to report a count mismatch.
    test("uniform shader myshader;", {},
         "The mesh specification declares 1 child effects, but the mesh supplies 0.");
    test("", childShader,
         "The mesh specification declares 0 child effects, but the mesh supplies 1.");

    // These are expected to report a type mismatch.
    test("uniform shader myshader;", childFilter,
         "Child effect 'myshader' was specified as a shader, but passed as a color filter.");
    test("uniform shader myshader;", childBlender,
         "Child effect 'myshader' was specified as a shader, but passed as a blender.");
    test("uniform colorFilter myfilter;", childShader,
         "Child effect 'myfilter' was specified as a color filter, but passed as a shader.");
    test("uniform colorFilter myfilter;", childBlender,
         "Child effect 'myfilter' was specified as a color filter, but passed as a blender.");
    test("uniform blender myblender;", childShader,
         "Child effect 'myblender' was specified as a blender, but passed as a shader.");
    test("uniform blender myblender;", childFilter,
         "Child effect 'myblender' was specified as a blender, but passed as a color filter.");

    // Null children are supported.
    test("uniform shader myshader;", childNull);
    test("uniform shader myfilter;", childNull);
    test("uniform shader myblender;", childNull);

    // Properly-typed child effects are supported.
    test("uniform shader myshader;", childShader);
    test("uniform colorFilter myfilter;", childFilter);
    test("uniform blender myblender;", childBlender);

}

DEF_TEST(MeshSpec_ValidUniforms, reporter) {
    using Uniform = SkMeshSpecification::Uniform;
    using Type    = Uniform::Type;
    using Flags   = Uniform::Flags;

    constexpr Flags kVS    = Uniform::kVertex_Flag;
    constexpr Flags kFS    = Uniform::kFragment_Flag;
    constexpr Flags kColor = Uniform::kColor_Flag;
    constexpr Flags kHalfP = Uniform::kHalfPrecision_Flag;

    auto make_uni = [](Type type,
                       std::string_view name,
                       size_t offset,
                       uint32_t flags,
                       int count = 0) {
        if (count) {
            return Uniform{name, offset, type, count, flags | Uniform::kArray_Flag};
        } else {
            SkASSERT(!(flags & Uniform::kArray_Flag));
            return Uniform{name, offset, type, 1, flags};
        }
    };

    // Each test case is a set of VS and FS uniform declarations followed and the expected output
    // of SkMeshSpecification::uniforms().
    struct {
        const std::vector<const char*>                  vsUniformDecls;
        const std::vector<const char*>                  fsUniformDecls;
        const std::vector<SkMeshSpecification::Uniform> expectations;
    } static kTestCases[] {
            // A single VS uniform.
            {
                    {
                            "uniform float x;"
                    },
                    {},
                    {
                            make_uni(Type::kFloat, "x", 0, kVS)
                    }
            },

            // A single FS uniform.
            {
                    {},
                    {
                            "uniform float2 v;"
                    },
                    {
                            make_uni(Type::kFloat2, "v", 0, kFS)
                    }
            },

            // A single uniform in both that uses color layout.
            {
                    {
                            "layout(color) uniform float4 color;",
                    },
                    {
                            "layout(color) uniform float4 color;",
                    },
                    {
                            make_uni(Type::kFloat4, "color", 0, kVS|kFS|kColor)
                    }
            },

            // A shared uniform after an unshared vertex uniform
            {
                    {
                            "layout(color) uniform float4 color;",
                            "              uniform float x[5];",
                    },
                    {
                            "uniform float x[5];",
                    },
                    {
                             make_uni(Type::kFloat4, "color",  0, kVS|kColor, 0),
                             make_uni(Type::kFloat , "x"    , 16, kVS|kFS   , 5)
                    }
            },

            // A shared uniform before an unshared vertex uniform
            {
                    {
                        "uniform half x[2];",
                        "uniform int  y;",
                    },
                    {
                        "uniform half x[2];",
                    },
                    {
                        make_uni(Type::kFloat, "x",  0, kVS|kFS|kHalfP, 2),
                        make_uni(Type::kInt,   "y",  8, kVS           , 0)
                    }
            },

            // A shared uniform after an unshared fragment uniform
            {
                    {
                            "uniform float3x3 m;",
                    },
                    {
                             "uniform int2     i2;",
                             "uniform float3x3 m;",
                    },
                    {
                            make_uni(Type::kFloat3x3, "m" ,  0, kVS|kFS),
                            make_uni(Type::kInt2    , "i2", 36, kFS    )
                    }
            },

            // A shared uniform before an unshared fragment uniform
            {
                    {
                            "uniform half4x4 m[4];",
                    },
                    {
                            "uniform half4x4  m[4];",
                            "uniform int3    i3[1];",
                    },
                    {
                            make_uni(Type::kFloat4x4, "m",    0, kVS|kFS|kHalfP, 4),
                            make_uni(Type::kInt3,     "i3", 256, kFS           , 1)
                    }
            },

            // Complex case with 2 shared uniforms that are declared in the opposite order.
            {
                    {
                             "uniform float   x;"
                             "uniform half4x4 m[4];",  // shared
                             "uniform int2    i2[2];"
                             "uniform float3  v[8];"   // shared
                             "uniform int3    i3;"
                    },
                    {
                             "uniform float   y;"
                             "uniform float3  v[8];"   // shared
                             "uniform int4    i4[2];"
                             "uniform half4x4 m[4];",  // shared
                             "uniform int     i;"
                    },
                    {
                             make_uni(Type::kFloat,    "x" ,   0, kVS           , 0),
                             make_uni(Type::kFloat4x4, "m" ,   4, kVS|kFS|kHalfP, 4),
                             make_uni(Type::kInt2,     "i2", 260, kVS           , 2),
                             make_uni(Type::kFloat3,   "v" , 276, kVS|kFS       , 8),
                             make_uni(Type::kInt3,     "i3", 372, kVS           , 0),
                             make_uni(Type::kFloat,    "y" , 384, kFS           , 0),
                             make_uni(Type::kInt4,     "i4", 388, kFS           , 2),
                             make_uni(Type::kInt,      "i" , 420, kFS           , 0),
                    }
            },
    };

    for (const auto& c : kTestCases) {
        SkString vs = kValidVS;
        SkString unis;
        for (const auto u : c.vsUniformDecls) {
            unis.append(u);
        }
        vs.prepend(unis);

        SkString fs = kValidFSes[0];
        unis = {};
        for (const auto u : c.fsUniformDecls) {
            unis.append(u);
        }
        fs.prepend(unis);

        auto attrs = SkSpan(kValidAttrs);
        auto varys = SkSpan(kValidVaryings);
        sk_sp<SkMeshSpecification> spec;
        if (!check_for_success(reporter, attrs, kValidStride, varys, vs, fs, &spec)) {
            return;
        }
        SkString desc = make_description(attrs, kValidStride, varys, vs, fs);
        SkSpan<const Uniform> uniforms = spec->uniforms();
        if (uniforms.size() != c.expectations.size()) {
            ERRORF(reporter,
                   "Expected %zu uniforms but actually %zu:\n%s",
                   c.expectations.size(),
                   uniforms.size(),
                   desc.c_str());
            return;
        }
        for (const auto& [actual, expected] : SkMakeZip(uniforms, c.expectations)) {
            std::string name = std::string(actual.name);
            if (name != expected.name) {
                ERRORF(reporter,
                       "Actual uniform name (%s) does not match expected name (%.*s)",
                       name.c_str(),
                       (int)expected.name.size(), expected.name.data());
                return;
            }
            if (actual.type != expected.type) {
                ERRORF(reporter,
                       "Uniform %s: Actual type (%d) does not match expected type (%d)",
                       name.c_str(),
                       static_cast<int>(actual.type),
                       static_cast<int>(expected.type));
                return;
            }
            if (actual.count != expected.count) {
                ERRORF(reporter,
                       "Uniform %s: Actual count (%d) does not match expected count (%d)",
                       name.c_str(),
                       actual.count,
                       expected.count);
                return;
            }
            if (actual.flags != expected.flags) {
                ERRORF(reporter,
                       "Uniform %s: Actual flags (0x%04x) do not match expected flags (0x%04x)",
                       name.c_str(),
                       actual.flags,
                       expected.flags);
                return;
            }
            if (actual.offset != expected.offset) {
                ERRORF(reporter,
                       "Uniform %s: Actual offset (%zu) does not match expected offset (%zu)",
                       name.c_str(),
                       actual.offset,
                       expected.offset);
                return;
            }
        }
    }
}

DEF_TEST(MeshSpec_InvalidUniforms, reporter) {
    // We assume general uniform declarations are broadly tested generically in SkSL. Here we are
    // concerned with agreement between VS and FS declarations, which is a unique aspect of
    // SkMeshSpecification.

    // Each test case is a fs and vs uniform declaration with the same name but some other
    // difference that should make them incompatible.
    static std::tuple<const char*, const char*> kTestCases[]{
            // different types
            {"uniform float x;", "uniform int x;"},
            // array vs non-array
            {"uniform float2x2 m[1];", "uniform float2x2 m;"},
            // array count mismatch
            {"uniform int3 i[1];", "uniform int3 i[2];"},
            // layout difference
            {"layout(color) uniform float4 color;", "uniform float4 color;"},
    };

    for (bool reverse : {false, true}) {
        for (auto [u1, u2] : kTestCases) {
            if (reverse) {
                using std::swap;
                swap(u1, u2);
            }
            SkString vs = kValidVS;
            vs.prepend(u1);

            SkString fs = kValidFSes[0];
            fs.prepend(u2);

            auto attrs = SkSpan(kValidAttrs);
            auto varys = SkSpan(kValidVaryings);
            if (!check_for_failure(reporter, attrs, kValidStride, varys, vs, fs)) {
                return;
            }
        }
    }
}

DEF_TEST(MeshSpec_MissingMain, reporter) {
    static const SkString kHelper{"float2 swiz(float2 x) { return z.yx; }"};

    // Empty VS
    if (!check_for_failure(reporter,
                           kValidAttrs,
                           kValidStride,
                           kValidVaryings,
                           SkString{},
                           kValidFSes[0])) {
        return;
    }

    // VS with helper function but no main
    if (!check_for_failure(reporter,
                           kValidAttrs,
                           kValidStride,
                           kValidVaryings,
                           kHelper,
                           kValidFSes[0])) {
        return;
    }

    // Empty FS
    if (!check_for_failure(reporter,
                           kValidAttrs,
                           kValidStride,
                           kValidVaryings,
                           kValidVS,
                           SkString{})) {
        return;
    }

    // VS with helper function but no main
    if (!check_for_failure(reporter,
                           kValidAttrs,
                           kValidStride,
                           kValidVaryings,
                           kValidVS,
                           kHelper)) {
        return;
    }
}

DEF_TEST(MeshSpec_ZeroAttributes, reporter) {
    // We require at least one attribute
    check_for_failure(reporter,
                      SkSpan<Attribute>(),
                      kValidStride,
                      kValidVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_ZeroVaryings, reporter) {
    // Varyings are not required.
    check_for_success(reporter,
                      kValidAttrs,
                      kValidStride,
                      SkSpan<Varying>(),
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_InvalidStride, reporter) {
    // Zero stride
    if (!check_for_failure(reporter,
                           kValidAttrs,
                           0,
                           kValidVaryings,
                           kValidVS,
                           kValidFSes[0])) {
        return;
    }

    // Unaligned
    if (!check_for_failure(reporter,
                           kValidAttrs,
                           kValidStride + 1,
                           kValidVaryings,
                           kValidVS,
                           kValidFSes[0])) {
        return;
    }

    // Too large
    if (!check_for_failure(reporter,
                           kValidAttrs,
                           1 << 20,
                           kValidVaryings,
                           kValidVS,
                           kValidFSes[0])) {
        return;
    }
}

DEF_TEST(MeshSpec_InvalidOffset, reporter) {
    {  // offset isn't aligned
        static const Attribute kAttributes[] {
                {Attribute::Type::kFloat4,  1, SkString{"var"}},
        };
        if (!check_for_failure(reporter,
                               kAttributes,
                               32,
                               kValidVaryings,
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
        if (!check_for_failure(reporter,
                               kAttributes,
                               20,
                               kValidVaryings,
                               kValidVS,
                               kValidFSes[0])) {
            return;
        }
    }
    {  // straddles stride boundary with attempt to overflow
        static const Attribute kAttributes[] {
                {Attribute::Type::kFloat, std::numeric_limits<size_t>::max() - 3, SkString{"var"}},
        };
        if (!check_for_failure(reporter,
                               kAttributes,
                               4,
                               kValidVaryings,
                               kValidVS,
                               kValidFSes[0])) {
            return;
        }
    }
}

DEF_TEST(MeshSpec_TooManyAttributes, reporter) {
    static constexpr size_t kN = 500;
    std::vector<Attribute> attrs;
    attrs.reserve(kN);
    for (size_t i = 0; i < kN; ++i) {
        attrs.push_back({Attribute::Type::kFloat4, 0, SkStringPrintf("attr%zu", i)});
    }
    check_for_failure(reporter,
                      attrs,
                      4*4,
                      kValidVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_TooManyVaryings, reporter) {
    static constexpr size_t kN = 500;
    std::vector<Varying> varyings;
    varyings.reserve(kN);
    for (size_t i = 0; i < kN; ++i) {
        varyings.push_back({Varying::Type::kFloat4, SkStringPrintf("varying%zu", i)});
    }
    check_for_failure(reporter,
                      kValidAttrs,
                      kValidStride,
                      SkSpan(varyings),
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_DuplicateAttributeNames, reporter) {
    static const Attribute kAttributes[] {
            {Attribute::Type::kFloat4,  0, SkString{"var"}},
            {Attribute::Type::kFloat2, 16, SkString{"var"}}
    };
    check_for_failure(reporter,
                      kAttributes,
                      24,
                      kValidVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_DuplicateVaryingNames, reporter) {
    static const Varying kVaryings[] {
        {Varying::Type::kFloat4, SkString{"var"}},
        {Varying::Type::kFloat3, SkString{"var"}}
    };
    check_for_failure(reporter,
                      kValidAttrs,
                      kValidStride,
                      kVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

static constexpr const char* kSneakyName = "name; float3 sneaky";

DEF_TEST(MeshSpec_SneakyExtraAttribute, reporter) {
    static const Attribute kAttributes[] {
            {Attribute::Type::kFloat4, 0, SkString{kSneakyName}},
    };
    check_for_failure(reporter,
                      kAttributes,
                      16,
                      kValidVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_SneakyExtraVarying, reporter) {
    static const Varying kVaryings[] {
            {Varying::Type::kFloat4, SkString{kSneakyName}},
    };
    check_for_failure(reporter,
                      kValidAttrs,
                      kValidStride,
                      kVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_AllowsFloat2PositionVarying, reporter) {
    // Position varying can be explicit if it is float2
    static const Varying kVaryings[] {
            {Varying::Type::kFloat2, SkString{"position"}},
    };
    check_for_success(reporter,
                      kValidAttrs,
                      kValidStride,
                      kVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_InvalidPositionType, reporter) {
    // Position varying can be explicit but it must be float2
    static const Varying kVaryings[] {
            {Varying::Type::kFloat4, SkString{"position"}},
    };
    check_for_failure(reporter,
                      kValidAttrs,
                      kValidStride,
                      kVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_EmptyAttributeName, reporter) {
    static const Attribute kAttributes[] {
            {Attribute::Type::kFloat4, 0, SkString{}},
    };
    check_for_failure(reporter,
                      kAttributes,
                      16,
                      kValidVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpec_EmptyVaryingName, reporter) {
    static const Varying kVaryings[] {
            {Varying::Type::kFloat4, SkString{}},
    };
    check_for_failure(reporter,
                      kValidAttrs,
                      kValidStride,
                      kVaryings,
                      kValidVS,
                      kValidFSes[0]);
}

DEF_TEST(MeshSpecVaryingPassthrough, reporter) {
    static const Attribute kAttributes[]{
            {Attribute::Type::kFloat2,        0, SkString{"position"}},
            {Attribute::Type::kFloat2,        8, SkString{"uv"}      },
            {Attribute::Type::kUByte4_unorm, 16, SkString{"color"}   },
    };
    static const Varying kVaryings[]{
            {Varying::Type::kFloat2, SkString{"position"}},
            {Varying::Type::kFloat2, SkString{"uv"}      },
            {Varying::Type::kHalf4,  SkString{"color"}   },
    };

    static constexpr char kVS[] = R"(
            Varyings main(const Attributes a) {
                Varyings v;
                v.uv       = a.uv;
                v.position = a.position;
                v.color    = a.color;
                return v;
            }
    )";
    auto check = [&] (const char* fs, const char* passthroughAttr) {
        auto [spec, error] = SkMeshSpecification::Make(kAttributes,
                                                       /*vertexStride=*/24,
                                                       kVaryings,
                                                       SkString(kVS),
                                                       SkString(fs));
        if (!spec) {
            ERRORF(reporter, "%s\n%s", fs, error.c_str());
            return;
        }
        int idx = SkMeshSpecificationPriv::PassthroughLocalCoordsVaryingIndex(*spec);
        const SkString& actualAttr = idx >= 0 ? spec->attributes()[idx].name : SkString("<none>");
        if (!passthroughAttr) {
            if (idx >= 0) {
                ERRORF(reporter, "Expected no passthrough coords attribute, found %s.\n%s",
                       actualAttr.c_str(),
                       fs);
            }
        } else if (!actualAttr.equals(passthroughAttr)) {
            ERRORF(reporter, "Expected %s as passthrough coords attribute, found %s.\n%s",
                   passthroughAttr,
                   actualAttr.c_str(),
                   fs);
        }
    };

    // Simple
    check(R"(float2 main(const Varyings v) {
                  return v.uv;
              })",
          "uv");

    // Simple, using position
    check(R"(float2 main(const Varyings v) {
                  return v.position;
              })",
          "position");

    // Simple, with output color
    check(R"(float2 main(const Varyings v, out half4 color) {
                  color = v.color;
                  return v.uv;
              })",
          "uv");

    // Three returns, all the same.
    check(R"(uniform int selector;

             float2 main(const Varyings v, out half4 color) {
                  if (selector == 0) {
                      color = half4(1, 0, 0, 1);
                      return v.position;
                  }
                  if (selector == 1) {
                      color = half4(1, 1, 0, 1);
                      return v.position;
                  }
                  color = half4(1, 0, 1, 1);
                  return v.position;
             })",
          "position");

    // Three returns, one not like the others
    check(R"(uniform int selector;

             float2 main(const Varyings v, out half4 color) {
                  if (selector == 0) {
                      color = color.bgra;
                      return v.position;
                  }
                  if (selector == 1) {
                      color = half4(1);
                      return v.uv;
                  }
                  color = color;
                  return v.position;
             })",
          nullptr);

    // Swizzles aren't handled (yet?).
    check(R"(float2 main(const Varyings v) {
                  return v.uv.yx;
              })",
          nullptr);

    // Return from non-main fools us?
    check(R"(noinline half4 get_color(const Varyings v) { return v.color; }

             float2 main(const Varyings v, out half4 color) {
                  color = get_color(v);
                  return v.position;
              })",
          "position");
}

DEF_TEST(MeshSpecUnusedVaryings, reporter) {
    static const Attribute kAttributes[]{
            {Attribute::Type::kFloat2,        0, SkString{"position"}},
            {Attribute::Type::kFloat2,        8, SkString{"uv"}      },
            {Attribute::Type::kUByte4_unorm, 16, SkString{"color"}   },
    };
    static const Varying kVaryings[]{
            {Varying::Type::kFloat2, SkString{"position"}},
            {Varying::Type::kFloat2, SkString{"uv"}      },
            {Varying::Type::kHalf4,  SkString{"color"}   },
    };

    static constexpr char kVS[] = R"(
            Varyings main(const Attributes a) {
                Varyings v;
                v.uv       = a.uv;
                v.position = a.position;
                v.color    = a.color;
                return v;
            }
    )";

    auto check = [&](const char* fs, bool positionDead, bool uvDead, bool colorDead) {
        static_assert(std::size(kVaryings) == 3);
        auto [spec, error] = SkMeshSpecification::Make(kAttributes,
                                                       /*vertexStride=*/24,
                                                       kVaryings,
                                                       SkString(kVS),
                                                       SkString(fs));
        if (!spec) {
            ERRORF(reporter, "%s\n%s", fs, error.c_str());
            return;
        }
        bool positionActuallyDead = SkMeshSpecificationPriv::VaryingIsDead(*spec, 0);
        bool uvActuallyDead       = SkMeshSpecificationPriv::VaryingIsDead(*spec, 1);
        bool colorActuallyDead    = SkMeshSpecificationPriv::VaryingIsDead(*spec, 2);
        auto str = [](bool dead) { return dead ? "dead" : "not dead"; };
        if (positionActuallyDead != positionDead) {
            ERRORF(reporter,
                   "Expected position to be detected %s but it is detected %s.\n%s",
                   str(positionDead),
                   str(positionActuallyDead),
                   fs);
        }
        if (uvActuallyDead != uvDead) {
            ERRORF(reporter,
                   "Expected uv to be detected %s but it is detected %s.\n%s",
                   str(uvDead),
                   str(uvActuallyDead),
                   fs);
        }
        if (colorActuallyDead != colorDead) {
            ERRORF(reporter,
                   "Expected color to be detected %s but it is detected %s.\n%s",
                   str(colorDead),
                   str(colorActuallyDead),
                   fs);
        }
    };

    // Simple
    check(R"(float2 main(const Varyings v) {
                 return v.uv;
             })",
          true,
          true,
          true);

    // Simple, using position
    check(R"(float2 main(const Varyings v) {
                 return v.position;
             })",
          true,
          true,
          true);

    // Two returns that are both passthrough of the same varying
    check(R"(float2 main(const Varyings v, out half4 color) {
                 if (v.color.r > 0.5) {
                     color = v.color;
                     return v.uv;
                 } else {
                     color = 2*color;
                     return v.uv;
                 }
             })",
          true,
          true,
          false);

    // Two returns that are both passthrough of the different varyings and unused other varying
    check(R"(float2 main(const Varyings v, out half4 color) {
                 if (v.position.x > 10) {
                     color = half4(0);
                     return v.uv;
                 } else {
                     color = half4(1);
                     return v.position;
                 }
             })",
          false,
          false,
          true);

    // Passthrough but we also use the varying elsewhere
    check(R"(float2 main(const Varyings v, out half4 color) {
                 color = half4(v.uv.x, 0, 0, 1);
                 return v.uv;
             })",
          true,
          false,
          true);

    // Use two varyings is a return statement
    check(R"(float2 main(const Varyings v) {
                  return v.uv + v.position;
              })",
          false,
          false,
          true);

    // Slightly more complicated varying use.
    check(R"(noinline vec2 get_pos(const Varyings v) { return v.position; }

             noinline half4 identity(half4 c) { return c; }

             float2 main(const Varyings v, out half4 color) {
                 color = identity(v.color);
                 return v.uv + get_pos(v);
             })",
          false,
          false,
          false);

    // Go through assignment to another Varyings.
    check(R"(float2 main(const Varyings v) {
                 Varyings otherVaryings;
                 otherVaryings = v;
                 return otherVaryings.uv;
             })",
          true,
          false,
          true);

    // We're not very smart. We just look for any use of the field in any Varyings value and don't
    // do any data flow analysis.
    check(R"(float2 main(const Varyings v) {
                 Varyings otherVaryings;
                 otherVaryings.uv       = half2(5);
                 otherVaryings.position = half2(10);
                 return otherVaryings.position;
             })",
          false,
          false,
          true);
}
