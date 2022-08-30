#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct sampler2D {
    texture2d<half> tex;
    sampler smp;
};
half4 sample(sampler2D i, float2 p, float b=0) { return i.tex.sample(i.smp, p, bias(b)); }
half4 sample(sampler2D i, float3 p, float b=0) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }

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
half4 TextureEffect_Stage1_c0_c0_c0_h4h4f2(thread Globals& _globals, half4 _input, float2 _coords) {
    float2 inCoord = _coords;
    inCoord *= _globals._anonInterface0->unorm_Stage1_c0_c0_c0.xy;
    float2 subsetCoord;
    subsetCoord.x = inCoord.x;
    subsetCoord.y = inCoord.y;
    float2 clampedCoord;
    clampedCoord = subsetCoord;
    half4 textureColor = sample(_globals.uTextureSampler_0_Stage1, clampedCoord * _globals._anonInterface0->unorm_Stage1_c0_c0_c0.zw);
    float snappedX = floor(inCoord.x + 0.0010000000474974513) + 0.5;
    if (snappedX < _globals._anonInterface0->usubset_Stage1_c0_c0_c0.x || snappedX > _globals._anonInterface0->usubset_Stage1_c0_c0_c0.z) {
        textureColor = _globals._anonInterface0->uborder_Stage1_c0_c0_c0;
    }
    return textureColor;
}
half4 MatrixEffect_Stage1_c0_c0_h4h4f2(thread Globals& _globals, half4 _input, float2 _coords) {
    return TextureEffect_Stage1_c0_c0_c0_h4h4f2(_globals, _input, (_globals._anonInterface0->umatrix_Stage1_c0_c0 * float3(_coords, 1.0)).xy);
}
half4 GaussianConvolution_Stage1_c0_h4h4(Inputs _in, thread Globals& _globals, half4 _input) {
    half4 _output;
    _output = half4(0.0h, 0.0h, 0.0h, 0.0h);
    float2 coord = _in.vLocalCoord_Stage0 - float2(12.0h * _globals._anonInterface0->uIncrement_Stage1_c0);
    float2 coordSampled = float2(0.0, 0.0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].x;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].y;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].z;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].w;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].x;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].y;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].z;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].w;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].x;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].y;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].z;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].w;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].x;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].y;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].z;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].w;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].x;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].y;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].z;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].w;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].x;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].y;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].z;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].w;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[6].x;
    coord += float2(_globals._anonInterface0->uIncrement_Stage1_c0);
    _output *= _input;
    return _output;
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
    half4 output_Stage1;
    output_Stage1 = GaussianConvolution_Stage1_c0_h4h4(_in, _globals, outputColor_Stage0);
    {
        _out.sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
    return _out;
}
