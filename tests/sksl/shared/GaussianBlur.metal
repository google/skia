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
float4 TextureEffect_Stage1_c0_c0_c0(thread Globals& _globals, float4 _input, float2 _coords) {
    float4 _output;
    float2 inCoord = _coords;
    inCoord *= _globals._anonInterface0->unorm_Stage1_c0_c0_c0.xy;
    float2 subsetCoord;
    subsetCoord.x = inCoord.x;
    subsetCoord.y = inCoord.y;
    float2 clampedCoord;
    clampedCoord = subsetCoord;
    float4 textureColor = _globals.uTextureSampler_0_Stage1.sample(_globals.uTextureSampler_0_Stage1Smplr, clampedCoord * _globals._anonInterface0->unorm_Stage1_c0_c0_c0.zw);
    float snappedX = floor(inCoord.x + 0.0010000000474974513) + 0.5;
    if (snappedX < _globals._anonInterface0->usubset_Stage1_c0_c0_c0.x || snappedX > _globals._anonInterface0->usubset_Stage1_c0_c0_c0.z) {
        textureColor = _globals._anonInterface0->uborder_Stage1_c0_c0_c0;
    }
    return textureColor;
}
float4 MatrixEffect_Stage1_c0_c0(thread Globals& _globals, float4 _input, float2 _coords) {
    float4 _output;
    return TextureEffect_Stage1_c0_c0_c0(_globals, _input, (_globals._anonInterface0->umatrix_Stage1_c0_c0 * float3(_coords, 1.0)).xy);
}
float4 GaussianConvolution_Stage1_c0(Inputs _in, thread Globals& _globals, float4 _input) {
    float4 _output;
    _output = float4(0.0, 0.0, 0.0, 0.0);
    float2 coord = _in.vLocalCoord_Stage0 - 12.0 * _globals._anonInterface0->uIncrement_Stage1_c0;
    float2 coordSampled = float2(0.0, 0.0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].x;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].y;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].z;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[0].w;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].x;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].y;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].z;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[1].w;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].x;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].y;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].z;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[2].w;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].x;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].y;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].z;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[3].w;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].x;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].y;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].z;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[4].w;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].x;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].y;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].z;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[5].w;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0(_globals, _input, coordSampled) * _globals._anonInterface0->uKernel_Stage1_c0[6].x;
    coord += _globals._anonInterface0->uIncrement_Stage1_c0;
    _output *= _input;
    return _output;
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
    output_Stage1 = GaussianConvolution_Stage1_c0(_in, _globals, outputColor_Stage0);
    {
        _out.sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
    return _out;
}
