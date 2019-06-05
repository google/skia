/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "src/sksl/SkSLCompiler.h"

class SkSLBench : public Benchmark {
public:
    SkSLBench(SkSL::String name, const char* src)
        : fName("sksl_" + name)
        , fSrc(src) {}

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            std::unique_ptr<SkSL::Program> program = fCompiler.convertProgram(
                                                                      SkSL::Program::kFragment_Kind,
                                                                      fSrc,
                                                                      fSettings);
            if (!fCompiler.errorCount()) {
                fCompiler.optimize(*program);
            } else {
                printf("%s\n", fCompiler.errorText().c_str());
                SK_ABORT("shader compilation failed");
            }
        }
    }

private:
    SkSL::String fName;
    SkSL::String fSrc;
    SkSL::Compiler fCompiler;
    SkSL::Program::Settings fSettings;

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH(return new SkSLBench("tiny", "void main() { sk_FragColor = half4(1); }"); )
DEF_BENCH(return new SkSLBench("huge", R"(
    uniform half2 uDstTextureUpperLeft_Stage1;
    uniform half2 uDstTextureCoordScale_Stage1;
    uniform sampler2D uDstTextureSampler_Stage1;
    noperspective in half4 vQuadEdge_Stage0;
    noperspective in half4 vinColor_Stage0;
    out half4 sk_FragColor;
    half luminance_Stage1(half3 color) {
        return dot(half3(0.3, 0.59, 0.11), color);
    }

    half3 set_luminance_Stage1(half3 hueSat, half alpha, half3 lumColor) {
        half diff = luminance_Stage1(lumColor - hueSat);
        half3 outColor = hueSat + diff;
        half outLum = luminance_Stage1(outColor);
        half minComp = min(min(outColor.r, outColor.g), outColor.b);
        half maxComp = max(max(outColor.r, outColor.g), outColor.b);
        if (minComp < 0.0 && outLum != minComp) {
            outColor = outLum + ((outColor - half3(outLum, outLum, outLum)) * outLum) /
                       (outLum - minComp);
        }
        if (maxComp > alpha && maxComp != outLum) {
            outColor = outLum +((outColor - half3(outLum, outLum, outLum)) * (alpha - outLum)) /
                       (maxComp - outLum);
        }
        return outColor;
    }

    void main() {
        half4 outputColor_Stage0;
        half4 outputCoverage_Stage0;
        { // Stage 0, QuadEdge
            outputColor_Stage0 = vinColor_Stage0;
            half edgeAlpha;
            half2 duvdx = half2(dFdx(vQuadEdge_Stage0.xy));
            half2 duvdy = half2(dFdy(vQuadEdge_Stage0.xy));
            if (vQuadEdge_Stage0.z > 0.0 && vQuadEdge_Stage0.w > 0.0) {
                edgeAlpha = min(min(vQuadEdge_Stage0.z, vQuadEdge_Stage0.w) + 0.5, 1.0);
            } else {
                half2 gF = half2(2.0 * vQuadEdge_Stage0.x * duvdx.x - duvdx.y,
                                 2.0 * vQuadEdge_Stage0.x * duvdy.x - duvdy.y);
                edgeAlpha = (vQuadEdge_Stage0.x*vQuadEdge_Stage0.x - vQuadEdge_Stage0.y);
                edgeAlpha = saturate(0.5 - edgeAlpha / length(gF));
            }
            outputCoverage_Stage0 = half4(edgeAlpha);
        }
        { // Xfer Processor: Custom Xfermode
            if (all(lessThanEqual(outputCoverage_Stage0.rgb, half3(0)))) {
                discard;
            }
            // Read color from copy of the destination.
            half2 _dstTexCoord = (half2(sk_FragCoord.xy) - uDstTextureUpperLeft_Stage1) *
                                  uDstTextureCoordScale_Stage1;
            _dstTexCoord.y = 1.0 - _dstTexCoord.y;
            half4 _dstColor = texture(uDstTextureSampler_Stage1, _dstTexCoord);
            sk_FragColor.a = outputColor_Stage0.a + (1.0 - outputColor_Stage0.a) * _dstColor.a;
            half4 srcDstAlpha = outputColor_Stage0 * _dstColor.a;
            sk_FragColor.rgb = set_luminance_Stage1(_dstColor.rgb * outputColor_Stage0.a,
                                                    srcDstAlpha.a, srcDstAlpha.rgb);
            sk_FragColor.rgb += (1.0 - outputColor_Stage0.a) * _dstColor.rgb + (1.0 - _dstColor.a) *
                                outputColor_Stage0.rgb;
            sk_FragColor = outputCoverage_Stage0 * sk_FragColor +
                           (half4(1.0) - outputCoverage_Stage0) * _dstColor;
        }
    }
)"); )
