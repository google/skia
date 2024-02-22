/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrPerlinNoise2Effect.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/shaders/SkPerlinNoiseShaderType.h"

#include <cstdint>
#include <iterator>

class SkShader;

/////////////////////////////////////////////////////////////////////
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrPerlinNoise2Effect)

#if defined(GR_TEST_UTILS)
std::unique_ptr<GrFragmentProcessor> GrPerlinNoise2Effect::TestCreate(GrProcessorTestData* d) {
    int numOctaves = d->fRandom->nextRangeU(2, 10);
    bool stitchTiles = d->fRandom->nextBool();
    SkScalar seed = SkIntToScalar(d->fRandom->nextU());
    SkISize tileSize;
    tileSize.fWidth = d->fRandom->nextRangeU(4, 4096);
    tileSize.fHeight = d->fRandom->nextRangeU(4, 4096);
    SkScalar baseFrequencyX = d->fRandom->nextRangeScalar(0.01f, 0.99f);
    SkScalar baseFrequencyY = d->fRandom->nextRangeScalar(0.01f, 0.99f);

    sk_sp<SkShader> shader(d->fRandom->nextBool()
                                   ? SkShaders::MakeFractalNoise(baseFrequencyX,
                                                                 baseFrequencyY,
                                                                 numOctaves,
                                                                 seed,
                                                                 stitchTiles ? &tileSize : nullptr)
                                   : SkShaders::MakeTurbulence(baseFrequencyX,
                                                               baseFrequencyY,
                                                               numOctaves,
                                                               seed,
                                                               stitchTiles ? &tileSize : nullptr));

    GrTest::TestAsFPArgs asFPArgs(d);
    return GrFragmentProcessors::Make(
            shader.get(), asFPArgs.args(), GrTest::TestMatrix(d->fRandom));
}
#endif

SkString GrPerlinNoise2Effect::Impl::emitHelper(EmitArgs& args) {
    const GrPerlinNoise2Effect& pne = args.fFp.cast<GrPerlinNoise2Effect>();

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    // Add noise function
    const GrShaderVar gPerlinNoiseArgs[] = {{"chanCoord", SkSLType::kHalf},
                                            {"noiseVec ", SkSLType::kHalf2}};

    const GrShaderVar gPerlinNoiseStitchArgs[] = {{"chanCoord", SkSLType::kHalf},
                                                  {"noiseVec", SkSLType::kHalf2},
                                                  {"stitchData", SkSLType::kHalf2}};

    SkString noiseCode;

    noiseCode.append(
            "half4 floorVal;"
            "floorVal.xy = floor(noiseVec);"
            "floorVal.zw = floorVal.xy + half2(1);"
            "half2 fractVal = fract(noiseVec);"

            // Hermite interpolation : t^2*(3 - 2*t)
            "half2 noiseSmooth = smoothstep(0, 1, fractVal);"
    );

    // Adjust frequencies if we're stitching tiles
    if (pne.stitchTiles()) {
        noiseCode.append("floorVal -= step(stitchData.xyxy, floorVal) * stitchData.xyxy;");
    }

    // NOTE: We need to explicitly pass half4(1) as input color here, because the helper function
    // can't see fInputColor (which is "_input" in the FP's outer function). skbug.com/10506
    SkString sampleX = this->invokeChild(0, "half4(1)", args, "half2(floorVal.x + 0.5, 0.5)");
    SkString sampleY = this->invokeChild(0, "half4(1)", args, "half2(floorVal.z + 0.5, 0.5)");
    noiseCode.appendf("half2 latticeIdx = half2(%s.a, %s.a);", sampleX.c_str(), sampleY.c_str());

    if (args.fShaderCaps->fPerlinNoiseRoundingFix) {
        // Android rounding for Tegra devices, like, for example: Xoom (Tegra 2), Nexus 7 (Tegra 3).
        // The issue is that colors aren't accurate enough on Tegra devices. For example, if an
        // 8 bit value of 124 (or 0.486275 here) is entered, we can get a texture value of
        // 123.513725 (or 0.484368 here). The following rounding operation prevents these precision
        // issues from affecting the result of the noise by making sure that we only have multiples
        // of 1/255. (Note that 1/255 is about 0.003921569, which is the value used here).
        noiseCode.append(
                "latticeIdx = floor(latticeIdx * half2(255.0) + half2(0.5)) * half2(0.003921569);");
    }

    // Get (x,y) coordinates with the permuted x
    noiseCode.append("half4 bcoords = 256*latticeIdx.xyxy + floorVal.yyww;");

    // This is the math to convert the two 16bit integer packed into rgba 8 bit input into a
    // [-1,1] vector and perform a dot product between that vector and the provided vector.
    // Save it as a string because we will repeat it 4x.
    static constexpr const char* inc8bit = "0.00390625";  // 1.0 / 256.0
    SkString dotLattice =
            SkStringPrintf("dot((lattice.ga + lattice.rb*%s)*2 - half2(1), fractVal)", inc8bit);

    SkString sampleA = this->invokeChild(1, "half4(1)", args, "half2(bcoords.x, chanCoord)");
    SkString sampleB = this->invokeChild(1, "half4(1)", args, "half2(bcoords.y, chanCoord)");
    SkString sampleC = this->invokeChild(1, "half4(1)", args, "half2(bcoords.w, chanCoord)");
    SkString sampleD = this->invokeChild(1, "half4(1)", args, "half2(bcoords.z, chanCoord)");

    // Compute u, at offset (0,0)
    noiseCode.appendf("half4 lattice = %s;", sampleA.c_str());
    noiseCode.appendf("half u = %s;", dotLattice.c_str());

    // Compute v, at offset (-1,0)
    noiseCode.append("fractVal.x -= 1.0;");
    noiseCode.appendf("lattice = %s;", sampleB.c_str());
    noiseCode.appendf("half v = %s;", dotLattice.c_str());

    // Compute 'a' as a linear interpolation of 'u' and 'v'
    noiseCode.append("half a = mix(u, v, noiseSmooth.x);");

    // Compute v, at offset (-1,-1)
    noiseCode.append("fractVal.y -= 1.0;");
    noiseCode.appendf("lattice = %s;", sampleC.c_str());
    noiseCode.appendf("v = %s;", dotLattice.c_str());

    // Compute u, at offset (0,-1)
    noiseCode.append("fractVal.x += 1.0;");
    noiseCode.appendf("lattice = %s;", sampleD.c_str());
    noiseCode.appendf("u = %s;", dotLattice.c_str());

    // Compute 'b' as a linear interpolation of 'u' and 'v'
    noiseCode.append("half b = mix(u, v, noiseSmooth.x);");

    // Compute the noise as a linear interpolation of 'a' and 'b'
    noiseCode.append("return mix(a, b, noiseSmooth.y);");

    SkString noiseFuncName = fragBuilder->getMangledFunctionName("noiseFuncName");
    if (pne.stitchTiles()) {
        fragBuilder->emitFunction(SkSLType::kHalf,
                                  noiseFuncName.c_str(),
                                  {gPerlinNoiseStitchArgs, std::size(gPerlinNoiseStitchArgs)},
                                  noiseCode.c_str());
    } else {
        fragBuilder->emitFunction(SkSLType::kHalf,
                                  noiseFuncName.c_str(),
                                  {gPerlinNoiseArgs, std::size(gPerlinNoiseArgs)},
                                  noiseCode.c_str());
    }

    return noiseFuncName;
}

void GrPerlinNoise2Effect::Impl::emitCode(EmitArgs& args) {
    SkString noiseFuncName = this->emitHelper(args);

    const GrPerlinNoise2Effect& pne = args.fFp.cast<GrPerlinNoise2Effect>();

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    fBaseFrequencyUni = uniformHandler->addUniform(
            &pne, kFragment_GrShaderFlag, SkSLType::kHalf2, "baseFrequency");
    const char* baseFrequencyUni = uniformHandler->getUniformCStr(fBaseFrequencyUni);

    const char* stitchDataUni = nullptr;
    if (pne.stitchTiles()) {
        fStitchDataUni = uniformHandler->addUniform(
                &pne, kFragment_GrShaderFlag, SkSLType::kHalf2, "stitchData");
        stitchDataUni = uniformHandler->getUniformCStr(fStitchDataUni);
    }

    // In the past, Perlin noise handled coordinates a bit differently than most shaders.
    // It operated in device space, floored; it also had a one-pixel transform matrix applied to
    // both the X and Y coordinates. This is roughly equivalent to adding 0.5 to the coordinates.
    // This was originally done in order to better match preexisting golden images from WebKit.
    // Perlin noise now operates in local space, which allows rotation to work correctly. To better
    // approximate past behavior, we add 0.5 to the coordinates here. This is _not_ the same because
    // this adjustment is occurring in local space, not device space, but it means that the "same"
    // noise will be calculated regardless of CTM.
    fragBuilder->codeAppendf(
            "half2 noiseVec = half2((%s + 0.5) * %s);", args.fSampleCoord, baseFrequencyUni);

    // Clear the color accumulator
    fragBuilder->codeAppendf("half4 color = half4(0);");

    if (pne.stitchTiles()) {
        fragBuilder->codeAppendf("half2 stitchData = %s;", stitchDataUni);
    }

    fragBuilder->codeAppendf("half ratio = 1.0;");

    // Loop over all octaves
    fragBuilder->codeAppendf("for (int octave = 0; octave < %d; ++octave) {", pne.numOctaves());
    fragBuilder->codeAppendf("color += ");
    if (pne.type() != SkPerlinNoiseShaderType::kFractalNoise) {
        fragBuilder->codeAppend("abs(");
    }

    // There are 4 lines, put y coords at center of each.
    static constexpr const char* chanCoordR = "0.5";
    static constexpr const char* chanCoordG = "1.5";
    static constexpr const char* chanCoordB = "2.5";
    static constexpr const char* chanCoordA = "3.5";
    if (pne.stitchTiles()) {
        fragBuilder->codeAppendf(
                "half4(%s(%s, noiseVec, stitchData), %s(%s, noiseVec, stitchData),"
                      "%s(%s, noiseVec, stitchData), %s(%s, noiseVec, stitchData))",
                noiseFuncName.c_str(),
                chanCoordR,
                noiseFuncName.c_str(),
                chanCoordG,
                noiseFuncName.c_str(),
                chanCoordB,
                noiseFuncName.c_str(),
                chanCoordA);
    } else {
        fragBuilder->codeAppendf(
                "half4(%s(%s, noiseVec), %s(%s, noiseVec),"
                      "%s(%s, noiseVec), %s(%s, noiseVec))",
                noiseFuncName.c_str(),
                chanCoordR,
                noiseFuncName.c_str(),
                chanCoordG,
                noiseFuncName.c_str(),
                chanCoordB,
                noiseFuncName.c_str(),
                chanCoordA);
    }
    if (pne.type() != SkPerlinNoiseShaderType::kFractalNoise) {
        fragBuilder->codeAppend(")");  // end of "abs("
    }
    fragBuilder->codeAppend(" * ratio;");

    fragBuilder->codeAppend(
            "noiseVec *= half2(2.0);"
            "ratio *= 0.5;");

    if (pne.stitchTiles()) {
        fragBuilder->codeAppend("stitchData *= half2(2.0);");
    }
    fragBuilder->codeAppend("}");  // end of the for loop on octaves

    if (pne.type() == SkPerlinNoiseShaderType::kFractalNoise) {
        // The value of turbulenceFunctionResult comes from ((turbulenceFunctionResult) + 1) / 2
        // by fractalNoise and (turbulenceFunctionResult) by turbulence.
        fragBuilder->codeAppendf("color = color * half4(0.5) + half4(0.5);");
    }

    // Clamp values
    fragBuilder->codeAppendf("color = saturate(color);");

    // Pre-multiply the result
    fragBuilder->codeAppendf("return half4(color.rgb * color.aaa, color.a);");
}

void GrPerlinNoise2Effect::Impl::onSetData(const GrGLSLProgramDataManager& pdman,
                                           const GrFragmentProcessor& processor) {
    const GrPerlinNoise2Effect& turbulence = processor.cast<GrPerlinNoise2Effect>();

    const SkVector& baseFrequency = turbulence.baseFrequency();
    pdman.set2f(fBaseFrequencyUni, baseFrequency.fX, baseFrequency.fY);

    if (turbulence.stitchTiles()) {
        const SkPerlinNoiseShader::StitchData& stitchData = turbulence.stitchData();
        pdman.set2f(fStitchDataUni,
                    SkIntToScalar(stitchData.fWidth),
                    SkIntToScalar(stitchData.fHeight));
    }
}

void GrPerlinNoise2Effect::onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const {
    uint32_t key = fNumOctaves;
    key = key << 3;  // Make room for next 3 bits
    switch (fType) {
        case SkPerlinNoiseShaderType::kFractalNoise:
            key |= 0x1;
            break;
        case SkPerlinNoiseShaderType::kTurbulence:
            key |= 0x2;
            break;
        default:
            // leave key at 0
            break;
    }
    if (fStitchTiles) {
        key |= 0x4;  // Flip the 3rd bit if tile stitching is on
    }
    b->add32(key);
}
