#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;

struct sampler2D {
    texture2d<half> tex;
    sampler smp;
};
half4 sample(sampler2D i, float2 p, float b=0) { return i.tex.sample(i.smp, p, bias(b)); }
half4 sample(sampler2D i, float3 p, float b=0) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }
half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }
half4 sampleLod(sampler2D i, float3 p, float lod) {
    return i.tex.sample(i.smp, p.xy / p.z, level(lod));
}
half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {
    return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));
}

struct Inputs {
    float2 vLocalCoord_Stage0  [[user(locn0)]];
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct uniformBuffer {
    float4 sk_RTAdjust;
    half2 uIncrement_Stage1_c0;
    char pad0[12];
    array<half4, 7> uKernel_Stage1_c0;
    char pad1[56];
    float3x3 umatrix_Stage1_c0_c0;
    half4 uborder_Stage1_c0_c0_c0;
    char pad2[8];
    float4 usubset_Stage1_c0_c0_c0;
    float4 unorm_Stage1_c0_c0_c0;
};
struct Globals {
    sampler2D uTextureSampler_0_Stage1;
    constant uniformBuffer* _anonInterface0;
};
half4 MatrixEffect_Stage1_c0_c0_h4h4f2(thread Globals& _globals, half4 _input, float2 _coords) {
    float2 _1_inCoord = (_globals._anonInterface0->umatrix_Stage1_c0_c0 * float3(_coords, 1.0)).xy;
    _1_inCoord *= _globals._anonInterface0->unorm_Stage1_c0_c0_c0.xy;
    float2 _2_subsetCoord;
    _2_subsetCoord.x = _1_inCoord.x;
    _2_subsetCoord.y = _1_inCoord.y;
    float2 _3_clampedCoord = _2_subsetCoord;
    half4 _4_textureColor = sample(_globals.uTextureSampler_0_Stage1, _3_clampedCoord * _globals._anonInterface0->unorm_Stage1_c0_c0_c0.zw);
    float _5_snappedX = floor(_1_inCoord.x + 0.001) + 0.5;
    if (_5_snappedX < _globals._anonInterface0->usubset_Stage1_c0_c0_c0.x || _5_snappedX > _globals._anonInterface0->usubset_Stage1_c0_c0_c0.z) {
        _4_textureColor = _globals._anonInterface0->uborder_Stage1_c0_c0_c0;
    }
    return _4_textureColor;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<half> uTextureSampler_0_Stage1_Tex [[texture(0)]], sampler uTextureSampler_0_Stage1_Smplr [[sampler(0)]], constant uniformBuffer& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{uTextureSampler_0_Stage1_Tex, uTextureSampler_0_Stage1_Smplr}, &_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    half4 outputColor_Stage0;
    half4 outputCoverage_Stage0;
    {
        outputColor_Stage0 = half4(1.0h);
        outputCoverage_Stage0 = half4(1.0h);
    }
    half4 _6_output = half4(0.0h);
    float2 _7_coord = _in.vLocalCoord_Stage0 - float2(12.0h * _globals._anonInterface0->uIncrement_Stage1_c0);
    float2 _8_coordSampled = float2(0.0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].x;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].y;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].z;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].w;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].x;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].y;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].z;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].w;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].x;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].y;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].z;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].w;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].x;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].y;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].z;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].w;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].x;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].y;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].z;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].w;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].x;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].y;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].z;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].w;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _8_coordSampled = _7_coord;
    _6_output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, outputColor_Stage0, _8_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[6].x;
    _7_coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _6_output *= outputColor_Stage0;
    half4 output_Stage1 = _6_output;
    {
        _out.sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
    return _out;
}
