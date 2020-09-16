/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLStringStream.h"

#include "tests/Test.h"

static void test(skiatest::Reporter* r, const GrShaderCaps& caps, const char* src,
                 std::vector<const char*> expectedH, std::vector<const char*> expectedCPP) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    settings.fRemoveDeadFunctions = false;
    SkSL::Compiler compiler;
    SkSL::StringStream output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kFragmentProcessor_Kind,
                                                             SkSL::String(src),
                                                             settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
        return;
    }
    REPORTER_ASSERT(r, program);
    bool success = compiler.toH(*program, "Test", output);
    if (!success) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, success);
    if (success) {
        for (const char* expected : expectedH) {
            bool found = strstr(output.str().c_str(), expected);
            if (!found) {
                SkDebugf("HEADER MISMATCH:\nsource:\n%s\n\n"
                         "header expected:\n'%s'\n\n"
                         "header received:\n'%s'",
                         src, expected, output.str().c_str());
            }
            REPORTER_ASSERT(r, found);
        }
    }
    output.reset();
    success = compiler.toCPP(*program, "Test", output);
    if (!success) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, success);
    if (success) {
        for (const char* expected : expectedCPP) {
            bool found = strstr(output.str().c_str(), expected);
            if (!found) {
                SkDebugf("CPP MISMATCH:\nsource:\n%s\n\n"
                         "cpp expected:\n'%s'\n\n"
                         "cpp received:\n'%s'",
                         src, expected, output.str().c_str());
            }
            REPORTER_ASSERT(r, found);
        }
    }
}

DEF_TEST(SkSLFPMainCoords, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             void main(float2 coord) {
                 sk_OutColor = half4(coord, coord);
             }
         )__SkSL__",
         /*expectedH=*/{
             "this->setUsesSampleCoordsDirectly();"
         },
         /*expectedCPP=*/{
            "fragBuilder->codeAppendf(\n"
            "R\"SkSL(%s = half4(%s, %s);\n"
            ")SkSL\"\n"
            ", args.fOutputColor, args.fSampleCoord, args.fSampleCoord);"
         });
}

DEF_TEST(SkSLFPLayoutWhen, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
            layout(when=someExpression(someOtherExpression())) uniform half sometimes;
            void main() {
            }
         )__SkSL__",
         /*expectedH=*/{},
         /*expectedCPP=*/{
            "if (someExpression(someOtherExpression())) {\n"
            "            sometimesVar = args.fUniformHandler->addUniform"
         });
}

DEF_TEST(SkSLFPChildProcessors, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor child1;
             in fragmentProcessor child2;
             void main() {
                 sk_OutColor = sample(child1) * sample(child2);
             }
         )__SkSL__",
         /*expectedH=*/{
            "this->registerChild(std::move(child1), SkSL::SampleUsage::PassThrough());",
            "this->registerChild(std::move(child2), SkSL::SampleUsage::PassThrough());"
         },
         /*expectedCPP=*/{
            "SkString _sample149 = this->invokeChild(0, args);\n",
            "SkString _sample166 = this->invokeChild(1, args);\n",
            "fragBuilder->codeAppendf(\n"
            "R\"SkSL(%s = %s * %s;\n"
            ")SkSL\"\n"
            ", args.fOutputColor, _sample149.c_str(), _sample166.c_str());",
            "this->cloneAndRegisterAllChildProcessors(src);",
         });
}

DEF_TEST(SkSLFPChildProcessorsWithInput, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             uniform half4 color;
             in fragmentProcessor child1;
             in fragmentProcessor child2;
             void main() {
                 half4 childIn = color;
                 half4 childOut1 = sample(child1, childIn);
                 half4 childOut2 = sample(child2, childOut1);
                 sk_OutColor = childOut2;
             }
         )__SkSL__",
         /*expectedH=*/{
            "this->registerChild(std::move(child1), SkSL::SampleUsage::PassThrough());",
            "this->registerChild(std::move(child2), SkSL::SampleUsage::PassThrough());"
         },
         /*expectedCPP=*/{
            "this->cloneAndRegisterAllChildProcessors(src);",
            R"__Cpp__(
        SkString _input227("childIn");
        SkString _sample227 = this->invokeChild(0, _input227.c_str(), args);
        fragBuilder->codeAppendf(
R"SkSL(
half4 childOut1 = %s;)SkSL"
, _sample227.c_str());
        SkString _input287("childOut1");
        SkString _sample287 = this->invokeChild(1, _input287.c_str(), args);
        fragBuilder->codeAppendf(
R"SkSL(
half4 childOut2 = %s;
%s = childOut2;
)SkSL"
, _sample287.c_str(), args.fOutputColor);
)__Cpp__"});
}

DEF_TEST(SkSLFPChildProcessorWithInputExpression, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             uniform half4 color;
             in fragmentProcessor child;
             void main() {
                 sk_OutColor = sample(child, color * half4(0.5));
             }
         )__SkSL__",
         /*expectedH=*/{
            "this->registerChild(std::move(child), SkSL::SampleUsage::PassThrough());",
         },
         /*expectedCPP=*/{
            "this->cloneAndRegisterAllChildProcessors(src);",
            R"__Cpp__(
        SkString _input140 = SkStringPrintf("%s * half4(0.5)", args.fUniformHandler->getUniformCStr(colorVar));
        SkString _sample140 = this->invokeChild(0, _input140.c_str(), args);
        fragBuilder->codeAppendf(
R"SkSL(%s = %s;
)SkSL"
, args.fOutputColor, _sample140.c_str());
)__Cpp__"});
}

DEF_TEST(SkSLFPChildFPAndGlobal, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor child;
             bool hasCap = sk_Caps.externalTextureSupport;
             void main() {
                 if (hasCap) {
                     sk_OutColor = sample(child);
                 } else {
                     sk_OutColor = half4(1);
                 }
             }
         )__SkSL__",
         /*expectedH=*/{
            "this->registerChild(std::move(child), SkSL::SampleUsage::PassThrough());"
         },
         /*expectedCPP=*/{
            "this->cloneAndRegisterAllChildProcessors(src);",
            R"__Cpp__(
hasCap = sk_Caps.externalTextureSupport;
        fragBuilder->codeAppendf(
R"SkSL(bool hasCap = %s;
if (hasCap) {)SkSL"
, (hasCap ? "true" : "false"));
        SkString _sample200 = this->invokeChild(0, args);
        fragBuilder->codeAppendf(
R"SkSL(
    %s = %s;
} else {
    %s = half4(1.0);
}
)SkSL"
, args.fOutputColor, _sample200.c_str(), args.fOutputColor);
)__Cpp__"});
}

DEF_TEST(SkSLFPChildProcessorInlineFieldAccess, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor child;
             void main() {
                 if (child.preservesOpaqueInput) {
                     sk_OutColor = sample(child);
                 } else {
                     sk_OutColor = half4(1);
                 }
             }
         )__SkSL__",
         /*expectedH=*/{
            "this->registerChild(std::move(child), SkSL::SampleUsage::PassThrough());"
         },
         /*expectedCPP=*/{
            "this->cloneAndRegisterAllChildProcessors(src);",
            R"__Cpp__(
        fragBuilder->codeAppendf(
R"SkSL(if (%s) {)SkSL"
, (_outer.childProcessor(0)->preservesOpaqueInput() ? "true" : "false"));
        SkString _sample161 = this->invokeChild(0, args);
        fragBuilder->codeAppendf(
R"SkSL(
    %s = %s;
} else {
    %s = half4(1.0);
}
)SkSL"
, args.fOutputColor, _sample161.c_str(), args.fOutputColor);
)__Cpp__"});
}

DEF_TEST(SkSLFPChildProcessorFieldAccess, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor child;
             bool opaque = child.preservesOpaqueInput;
             void main() {
                 if (opaque) {
                     sk_OutColor = sample(child);
                 } else {
                     sk_OutColor = half4(0.5);
                 }
         }
         )__SkSL__",
         /*expectedH=*/{
            "this->registerChild(std::move(child), SkSL::SampleUsage::PassThrough());"
         },
         /*expectedCPP=*/{
            "opaque = _outer.childProcessor(0)->preservesOpaqueInput();",
            "fragBuilder->codeAppendf(\n"
            "R\"SkSL(bool opaque = %s;\n"
            "if (opaque) {)SkSL\"\n"
            ", (opaque ? \"true\" : \"false\"));",
            "SkString _sample196 = this->invokeChild(0, args);",
            "fragBuilder->codeAppendf(\n"
            "R\"SkSL(\n"
            "    %s = %s;\n"
            "} else {\n"
            "    %s = half4(0.5);\n"
            "}\n"
            ")SkSL\"\n"
            ", args.fOutputColor, _sample196.c_str(), args.fOutputColor);",
            "this->cloneAndRegisterAllChildProcessors(src);",
         });
}

DEF_TEST(SkSLFPSampleCoords, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor child;
             void main(float2 coord) {
                 sk_OutColor = sample(child) + sample(child, coord / 2);
             }
         )__SkSL__",
         /*expectedH=*/{
             "this->registerChild(std::move(child), SkSL::SampleUsage(SkSL::SampleUsage::Kind::kNone, \"\", false, true, true));",
             "this->setUsesSampleCoordsDirectly();"
         },
         /*expectedCPP=*/{
            "SkString _sample118 = this->invokeChild(0, args);\n",
            "SkString _coords134 = SkStringPrintf(\"%s / 2.0\", args.fSampleCoord);\n",
            "SkString _sample134 = this->invokeChild(0, args, _coords134.c_str());\n",
            "fragBuilder->codeAppendf(\n"
            "R\"SkSL(%s = %s + %s;\n"
            ")SkSL\"\n"
            ", args.fOutputColor, _sample118.c_str(), _sample134.c_str());"
        });
}

DEF_TEST(SkSLFPSwitchWithMultipleReturnsInside, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             uniform half4 color;
             half4 switchy(half4 c) {
                 switch (int(c.x)) {
                     case 0: return c.yyyy;
                     default: return c.zzzz;
                 }
             }
             void main() {
                 sk_OutColor = switchy(color);
             }
         )__SkSL__",
         /*expectedH=*/{},
         /*expectedCPP=*/{
         R"__Cpp__(fragBuilder->emitFunction(kHalf4_GrSLType, "switchy", 1, switchy_args,
R"SkSL(switch (int(c.x)) {
    case 0:
        return c.yyyy;
    default:
        return c.zzzz;
}
)SkSL", &switchy_name);
        fragBuilder->codeAppendf(
R"SkSL(%s = %s(%s);
)SkSL"
, args.fOutputColor, switchy_name.c_str(), args.fUniformHandler->getUniformCStr(colorVar));
)__Cpp__"});
}

DEF_TEST(SkSLFPGrSLTypesAreSupported, r) {
    // We thwart the optimizer by wrapping our return statement in a loop, which prevents inlining.
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             int test(int a) { for (;;) { return a; } }
             void main() { sk_OutColor = test(1).xxxx; }
         )__SkSL__",
         /*expectedH=*/{},
         /*expectedCPP=*/{
            R"__Cpp__(const GrShaderVar test_args[] = { GrShaderVar("a", kInt_GrSLType)};)__Cpp__",
            R"__Cpp__(fragBuilder->emitFunction(kInt_GrSLType, "test", 1, test_args,)__Cpp__",
         });
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             int2 test(int2 a) { for (;;) { return a; } }
             void main() { sk_OutColor = test(int2(1)).xyxy; }
         )__SkSL__",
         /*expectedH=*/{},
         /*expectedCPP=*/{
            R"__Cpp__(const GrShaderVar test_args[] = { GrShaderVar("a", kInt2_GrSLType)};)__Cpp__",
            R"__Cpp__(fragBuilder->emitFunction(kInt2_GrSLType, "test", 1, test_args,)__Cpp__",
         });
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             int3 test(int3 a) { for (;;) { return a; } }
             void main() { sk_OutColor = test(int3(1)).xyzx; }
         )__SkSL__",
         /*expectedH=*/{},
         /*expectedCPP=*/{
            R"__Cpp__(const GrShaderVar test_args[] = { GrShaderVar("a", kInt3_GrSLType)};)__Cpp__",
            R"__Cpp__(fragBuilder->emitFunction(kInt3_GrSLType, "test", 1, test_args,)__Cpp__",
         });
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             int4 test(int4 a) { for (;;) { return a; } }
             void main() { sk_OutColor = test(int4(1)); }
         )__SkSL__",
         /*expectedH=*/{},
         /*expectedCPP=*/{
            R"__Cpp__(const GrShaderVar test_args[] = { GrShaderVar("a", kInt4_GrSLType)};)__Cpp__",
            R"__Cpp__(fragBuilder->emitFunction(kInt4_GrSLType, "test", 1, test_args,)__Cpp__",
         });
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             half3x4 test(float3x4 a) { for (;;) { return half3x4(a); } }
             void main() { sk_OutColor = test(float3x4(0))[0]; }
         )__SkSL__",
         /*expectedH=*/{},
         /*expectedCPP=*/{
            R"__Cpp__(const GrShaderVar test_args[] = { GrShaderVar("a", kFloat3x4_GrSLType)};)__Cpp__",
            R"__Cpp__(fragBuilder->emitFunction(kHalf3x4_GrSLType, "test", 1, test_args,)__Cpp__",
         });
}

DEF_TEST(SkSLFPMatrixSampleConstant, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor? child;
             void main() {
                 sk_OutColor = sample(child, float3x3(2));
             }
         )__SkSL__",
         /*expectedH=*/{
             "this->registerChild(std::move(child), "
                    "SkSL::SampleUsage::UniformMatrix(\"float3x3(2.0)\", true));"
         },
         /*expectedCPP=*/{
             "this->invokeChildWithMatrix(0, args)"
         });
}

DEF_TEST(SkSLFPMatrixSampleUniform, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor? child;
             uniform float3x3 matrix;
             void main() {
                 sk_OutColor = sample(child, matrix);
             }
         )__SkSL__",
         /*expectedH=*/{
             // Since 'matrix' is just a uniform, the generated code can't determine perspective.
             "this->registerChild(std::move(child), "
                    "SkSL::SampleUsage::UniformMatrix(\"matrix\", true));"
         },
         /*expectedCPP=*/{
             "this->invokeChildWithMatrix(0, args)"
         });
}

DEF_TEST(SkSLFPMatrixSampleInUniform, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor? child;
             in uniform float3x3 matrix;
             void main() {
                 sk_OutColor = sample(child, matrix);
             }
         )__SkSL__",
         /*expectedH=*/{
             // Since 'matrix' is marked 'in', we can detect perspective at runtime
             "this->registerChild(std::move(child), "
                    "SkSL::SampleUsage::UniformMatrix(\"matrix\", matrix.hasPerspective()));"
         },
         /*expectedCPP=*/{
             "this->invokeChildWithMatrix(0, args)"
         });
}

DEF_TEST(SkSLFPMatrixSampleMultipleInUniforms, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor? child;
             in uniform float3x3 matrixA;
             in uniform float3x3 matrixB;
             void main() {
                 sk_OutColor = sample(child, matrixA);
                 sk_OutColor += sample(child, matrixB);
             }
         )__SkSL__",
         /*expectedH=*/{
             // FIXME it would be nice if codegen can produce
             // (matrixA.hasPerspective() || matrixB.hasPerspective()) even though it's variable.
             "this->registerChild(std::move(child), "
                    "SkSL::SampleUsage::VariableMatrix(true));"
         },
         /*expectedCPP=*/{
             "SkString _matrix191(args.fUniformHandler->getUniformCStr(matrixAVar));",
             "this->invokeChildWithMatrix(0, args, _matrix191.c_str());",
             "SkString _matrix247(args.fUniformHandler->getUniformCStr(matrixBVar));",
             "this->invokeChildWithMatrix(0, args, _matrix247.c_str());"
         });
}

DEF_TEST(SkSLFPMatrixSampleConstUniformExpression, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor? child;
             uniform float3x3 matrix;
             void main() {
                 sk_OutColor = sample(child, 0.5 * matrix);
             }
         )__SkSL__",
         /*expectedH=*/{
             // FIXME: "0.5 * matrix" is a uniform expression and could be lifted to the vertex
             // shader, once downstream code is able to properly map 'matrix' within the expression.
             "this->registerChild(std::move(child), "
                    "SkSL::SampleUsage::VariableMatrix(true));"
         },
         /*expectedCPP=*/{
            "SkString _matrix145 = SkStringPrintf(\"0.5 * %s\", "
                    "args.fUniformHandler->getUniformCStr(matrixVar));",
             "this->invokeChildWithMatrix(0, args, _matrix145.c_str());"
         });
}

DEF_TEST(SkSLFPMatrixSampleConstantAndExplicitly, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in fragmentProcessor? child;
             void main(float2 coord) {
                 sk_OutColor = sample(child, float3x3(0.5));
                 sk_OutColor = sample(child, coord / 2);
             }
         )__SkSL__",
         /*expectedH=*/{
             "this->registerChild(std::move(child), "
                    "SkSL::SampleUsage(SkSL::SampleUsage::Kind::kUniform, \"float3x3(0.5)\", true, true, false));"
         },
         /*expectedCPP=*/{
             "this->invokeChildWithMatrix(0, args)",
             "SkString _coords180 = SkStringPrintf(\"%s / 2.0\", args.fSampleCoord);",
             "this->invokeChild(0, args, _coords180.c_str())",
         });
}

DEF_TEST(SkSLFPMatrixSampleVariableAndExplicitly, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             uniform half4 color;
             in fragmentProcessor? child;
             void main(float2 coord) {
                 float3x3 matrix = float3x3(color.a);
                 sk_OutColor = sample(child, matrix);
                 sk_OutColor = sample(child, coord / 2);
             }
         )__SkSL__",
         /*expectedH=*/{
             "this->registerChild(std::move(child), "
                    "SkSL::SampleUsage(SkSL::SampleUsage::Kind::kVariable, \"\", true, true, false));"
         },
         /*expectedCPP=*/{
             R"__Cpp__(
        colorVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag, kHalf4_GrSLType, "color");
        fragBuilder->codeAppendf(
R"SkSL(float3x3 matrix = float3x3(float(%s.w));)SkSL"
, args.fUniformHandler->getUniformCStr(colorVar));
        SkString _matrix207("matrix");
        SkString _sample207 = this->invokeChildWithMatrix(0, args, _matrix207.c_str());
        fragBuilder->codeAppendf(
R"SkSL(
%s = %s;)SkSL"
, args.fOutputColor, _sample207.c_str());
        SkString _coords261 = SkStringPrintf("%s / 2.0", args.fSampleCoord);
        SkString _sample261 = this->invokeChild(0, args, _coords261.c_str());
        fragBuilder->codeAppendf(
R"SkSL(
%s = %s;
)SkSL"
, args.fOutputColor, _sample261.c_str());
)__Cpp__"
         });
}

DEF_TEST(SkSLUniformArrays, r) {
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             uniform half scalarArray[4];
             uniform half2 pointArray[2];
             void main() {
                sk_OutColor = half4(scalarArray[0] * pointArray[0].x +
                                    scalarArray[1] * pointArray[0].y +
                                    scalarArray[2] * pointArray[1].x +
                                    scalarArray[3] * pointArray[1].y);
             }
         )__SkSL__",
         /*expectedH=*/{
             "Make()",
         },
         /*expectedCPP=*/{
             "void onSetData(const GrGLSLProgramDataManager& pdman, "
             "const GrFragmentProcessor& _proc) override {\n    }"
         });
    test(r,
         *SkSL::ShaderCapsFactory::Default(),
         R"__SkSL__(
             in uniform half scalarArray[4];
             in uniform half2 pointArray[2];
             void main() {
                sk_OutColor = half4(scalarArray[0] * pointArray[0].x +
                                    scalarArray[1] * pointArray[0].y +
                                    scalarArray[2] * pointArray[1].x +
                                    scalarArray[3] * pointArray[1].y);
             }
         )__SkSL__",
         /*expectedH=*/{
             "Make(std::array<float> scalarArray, std::array<SkPoint> pointArray)",
             "std::array<float> scalarArray;",
             "std::array<SkPoint> pointArray;",
         },
         /*expectedCPP=*/{
             "pdman.set1fv(scalarArrayVar, 4, &(_outer.scalarArray)[0]);",
             "pdman.set2fv(pointArrayVar, 2, &pointArrayValue[0].fX);",
         });
}
