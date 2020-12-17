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
    float4 uKernel_Stage1_c0[7];
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


float4 MatrixEffect_Stage1_c0_c0(thread Globals* _globals, float4 _input, float2 _coords) {
    float2 _1_coords = (_globals->_anonInterface0->umatrix_Stage1_c0_c0 * float3(_coords, 1.0)).xy;
    float2 _2_inCoord = _1_coords;
    _2_inCoord *= _globals->_anonInterface0->unorm_Stage1_c0_c0_c0.xy;
    float2 _3_subsetCoord;
    _3_subsetCoord.x = _2_inCoord.x;
    _3_subsetCoord.y = _2_inCoord.y;
    float2 _4_clampedCoord;
    _4_clampedCoord = _3_subsetCoord;
    float4 _5_textureColor = _globals->uTextureSampler_0_Stage1.sample(_globals->uTextureSampler_0_Stage1Smplr, _4_clampedCoord * _globals->_anonInterface0->unorm_Stage1_c0_c0_c0.zw);
    float _6_snappedX = floor(_2_inCoord.x + 0.0010000000474974513) + 0.5;
    if (_6_snappedX < _globals->_anonInterface0->usubset_Stage1_c0_c0_c0.x || _6_snappedX > _globals->_anonInterface0->usubset_Stage1_c0_c0_c0.z) {
        _5_textureColor = _globals->_anonInterface0->uborder_Stage1_c0_c0_c0;
    }
    return _5_textureColor;

}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], texture2d<float> uTextureSampler_0_Stage1[[texture(0)]], sampler uTextureSampler_0_Stage1Smplr[[sampler(0)]], constant uniformBuffer& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{&_anonInterface0, uTextureSampler_0_Stage1, uTextureSampler_0_Stage1Smplr};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 output_Stage1;
    float4 _8_output;
    _8_output = float4(0.0, 0.0, 0.0, 0.0);
    float2 _9_coord = _in.vLocalCoord_Stage0 - 12.0 * _globals->_anonInterface0->uIncrement_Stage1_c0;
    float2 _10_coordSampled = float2(0.0, 0.0);
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[0].x;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[0].y;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[0].z;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[0].w;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[1].x;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[1].y;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[1].z;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[1].w;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[2].x;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[2].y;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[2].z;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[2].w;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[3].x;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[3].y;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[3].z;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[3].w;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[4].x;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[4].y;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[4].z;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[4].w;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[5].x;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[5].y;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[5].z;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[5].w;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    _10_coordSampled = _9_coord;
    _8_output += MatrixEffect_Stage1_c0_c0(_globals, float4(1.0), _10_coordSampled) * _globals->_anonInterface0->uKernel_Stage1_c0[6].x;
    _9_coord += _globals->_anonInterface0->uIncrement_Stage1_c0;
    output_Stage1 = _8_output;

    {
        _out->sk_FragColor = output_Stage1;
    }
    return *_out;
}
