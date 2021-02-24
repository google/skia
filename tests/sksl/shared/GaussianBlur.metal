#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float2 vLocalCoord_Stage0  [[user(locn0)]];
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct uniformBuffer {
    float4 sk_RTAdjust;
    float2 uIncrement_Stage1_c0;
    char pad0[8];
    array<float4, 7> uKernel_Stage1_c0;
    float3x3 umatrix_Stage1_c0_c0;
    float4 uborder_Stage1_c0_c0_c0;
    float4 usubset_Stage1_c0_c0_c0;
    float4 unorm_Stage1_c0_c0_c0;
};
struct Globals {
    constant uniformBuffer* _anonInterface0;
    texture2d<float> uTextureSampler_0_Stage1;
    sampler uTextureSampler_0_Stage1Smplr;
};


float4 MatrixEffect_Stage1_c0_c0(thread Globals& _globals, float4 _input, float2 _coords) {
    float4 _output;
    float4 _0_TextureEffect_Stage1_c0_c0_c0;
    float2 _1_coords = (_globals._anonInterface0->umatrix_Stage1_c0_c0 * float3(_coords, 1.0)).xy;
    float4 _2_output;
    float2 _3_inCoord = _1_coords;
    _3_inCoord *= _globals._anonInterface0->unorm_Stage1_c0_c0_c0.xy;
    float2 _4_subsetCoord;
    _4_subsetCoord.x = _3_inCoord.x;
    _4_subsetCoord.y = _3_inCoord.y;
    float2 _5_clampedCoord;
    _5_clampedCoord = _4_subsetCoord;
    float4 _6_textureColor = _globals.uTextureSampler_0_Stage1.sample(_globals.uTextureSampler_0_Stage1Smplr, _5_clampedCoord * _globals._anonInterface0->unorm_Stage1_c0_c0_c0.zw);
    float _7_snappedX = floor(_3_inCoord.x + 0.0010000000474974513) + 0.5;
    if (_7_snappedX < _globals._anonInterface0->usubset_Stage1_c0_c0_c0.x || _7_snappedX > _globals._anonInterface0->usubset_Stage1_c0_c0_c0.z) {
        _6_textureColor = _globals._anonInterface0->uborder_Stage1_c0_c0_c0;
    }
    return _6_textureColor;

}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<float> uTextureSampler_0_Stage1[[texture(0)]], sampler uTextureSampler_0_Stage1Smplr[[sampler(0)]], constant uniformBuffer& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0, uTextureSampler_0_Stage1, uTextureSampler_0_Stage1Smplr};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float4 outputColor_Stage0;
    float4 outputCoverage_Stage0;
    {
        outputColor_Stage0 = float4(1.0);
        outputCoverage_Stage0 = float4(1.0);
    }
    float4 output_Stage1;
    float4 _8_GaussianConvolution_Stage1_c0;
    float4 _9_output;
    _9_output = float4(0.0, 0.0, 0.0, 0.0);
    float2 _10_coord = _in.vLocalCoord_Stage0 - 12.0 * _globals._anonInterface0->uIncrement_Stage1_c0;
    float2 _11_coordSampled = float2(0.0, 0.0);
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].x;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].y;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].z;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].w;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].x;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].y;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].z;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].w;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].x;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].y;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].z;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].w;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].x;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].y;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].z;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].w;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].x;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].y;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].z;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].w;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].x;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].y;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].z;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].w;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _11_coordSampled = _10_coord;
    _9_output += MatrixEffect_Stage1_c0_c0(_globals, outputColor_Stage0, _11_coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[6].x;
    _10_coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _9_output *= outputColor_Stage0;
    output_Stage1 = _9_output;

    {
        _out.sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
    return _out;
}
