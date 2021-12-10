cbuffer uniformBuffer : register(b0, space0)
{
    float4 _4_sk_RTAdjust : packoffset(c0);
    float2 _4_uIncrement_Stage1_c0 : packoffset(c1);
    float4 _4_uKernel_Stage1_c0[7] : packoffset(c2);
    row_major float3x3 _4_umatrix_Stage1_c0_c0 : packoffset(c9);
    float4 _4_uborder_Stage1_c0_c0_c0 : packoffset(c12);
    float4 _4_usubset_Stage1_c0_c0_c0 : packoffset(c13);
    float4 _4_unorm_Stage1_c0_c0_c0 : packoffset(c14);
};

Texture2D<float4> uTextureSampler_0_Stage1 : register(t0, space0);
SamplerState _uTextureSampler_0_Stage1_sampler : register(s0, space0);

static float4 sk_FragColor;
static float2 vLocalCoord_Stage0;

struct SPIRV_Cross_Input
{
    float2 vLocalCoord_Stage0 : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 MatrixEffect_Stage1_c0_c0_h4h4f2(float4 _29, float2 _30)
{
    float2 _RESERVED_IDENTIFIER_FIXUP_1_inCoord = mul(float3(_30, 1.0f), _4_umatrix_Stage1_c0_c0).xy;
    _RESERVED_IDENTIFIER_FIXUP_1_inCoord *= _4_unorm_Stage1_c0_c0_c0.xy;
    float2 _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord = 0.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord.x = _RESERVED_IDENTIFIER_FIXUP_1_inCoord.x;
    _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord.y = _RESERVED_IDENTIFIER_FIXUP_1_inCoord.y;
    float2 _RESERVED_IDENTIFIER_FIXUP_3_clampedCoord = _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord;
    float4 _RESERVED_IDENTIFIER_FIXUP_4_textureColor = uTextureSampler_0_Stage1.Sample(_uTextureSampler_0_Stage1_sampler, _RESERVED_IDENTIFIER_FIXUP_3_clampedCoord * _4_unorm_Stage1_c0_c0_c0.zw);
    float _RESERVED_IDENTIFIER_FIXUP_5_snappedX = floor(_RESERVED_IDENTIFIER_FIXUP_1_inCoord.x + 0.001000000047497451305389404296875f) + 0.5f;
    bool _93 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_5_snappedX < _4_usubset_Stage1_c0_c0_c0.x)
    {
        _93 = true;
    }
    else
    {
        _93 = _RESERVED_IDENTIFIER_FIXUP_5_snappedX > _4_usubset_Stage1_c0_c0_c0.z;
    }
    if (_93)
    {
        _RESERVED_IDENTIFIER_FIXUP_4_textureColor = _4_uborder_Stage1_c0_c0_c0;
    }
    return _RESERVED_IDENTIFIER_FIXUP_4_textureColor;
}

void frag_main()
{
    float4 outputColor_Stage0 = 1.0f.xxxx;
    float4 outputCoverage_Stage0 = 1.0f.xxxx;
    float4 _RESERVED_IDENTIFIER_FIXUP_6_output = 0.0f.xxxx;
    float2 _RESERVED_IDENTIFIER_FIXUP_7_coord = vLocalCoord_Stage0 - (_4_uIncrement_Stage1_c0 * 12.0f);
    float2 _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = 0.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _123 = outputColor_Stage0;
    float2 _125 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_123, _125) * _4_uKernel_Stage1_c0[0].x);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _140 = outputColor_Stage0;
    float2 _142 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_140, _142) * _4_uKernel_Stage1_c0[0].y);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _156 = outputColor_Stage0;
    float2 _158 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_156, _158) * _4_uKernel_Stage1_c0[0].z);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _172 = outputColor_Stage0;
    float2 _174 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_172, _174) * _4_uKernel_Stage1_c0[0].w);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _188 = outputColor_Stage0;
    float2 _190 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_188, _190) * _4_uKernel_Stage1_c0[1].x);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _204 = outputColor_Stage0;
    float2 _206 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_204, _206) * _4_uKernel_Stage1_c0[1].y);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _220 = outputColor_Stage0;
    float2 _222 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_220, _222) * _4_uKernel_Stage1_c0[1].z);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _236 = outputColor_Stage0;
    float2 _238 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_236, _238) * _4_uKernel_Stage1_c0[1].w);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _252 = outputColor_Stage0;
    float2 _254 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_252, _254) * _4_uKernel_Stage1_c0[2].x);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _268 = outputColor_Stage0;
    float2 _270 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_268, _270) * _4_uKernel_Stage1_c0[2].y);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _284 = outputColor_Stage0;
    float2 _286 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_284, _286) * _4_uKernel_Stage1_c0[2].z);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _300 = outputColor_Stage0;
    float2 _302 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_300, _302) * _4_uKernel_Stage1_c0[2].w);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _316 = outputColor_Stage0;
    float2 _318 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_316, _318) * _4_uKernel_Stage1_c0[3].x);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _332 = outputColor_Stage0;
    float2 _334 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_332, _334) * _4_uKernel_Stage1_c0[3].y);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _348 = outputColor_Stage0;
    float2 _350 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_348, _350) * _4_uKernel_Stage1_c0[3].z);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _364 = outputColor_Stage0;
    float2 _366 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_364, _366) * _4_uKernel_Stage1_c0[3].w);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _380 = outputColor_Stage0;
    float2 _382 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_380, _382) * _4_uKernel_Stage1_c0[4].x);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _396 = outputColor_Stage0;
    float2 _398 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_396, _398) * _4_uKernel_Stage1_c0[4].y);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _412 = outputColor_Stage0;
    float2 _414 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_412, _414) * _4_uKernel_Stage1_c0[4].z);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _428 = outputColor_Stage0;
    float2 _430 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_428, _430) * _4_uKernel_Stage1_c0[4].w);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _444 = outputColor_Stage0;
    float2 _446 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_444, _446) * _4_uKernel_Stage1_c0[5].x);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _460 = outputColor_Stage0;
    float2 _462 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_460, _462) * _4_uKernel_Stage1_c0[5].y);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _476 = outputColor_Stage0;
    float2 _478 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_476, _478) * _4_uKernel_Stage1_c0[5].z);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _492 = outputColor_Stage0;
    float2 _494 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_492, _494) * _4_uKernel_Stage1_c0[5].w);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _RESERVED_IDENTIFIER_FIXUP_7_coord;
    float4 _508 = outputColor_Stage0;
    float2 _510 = _RESERVED_IDENTIFIER_FIXUP_8_coordSampled;
    _RESERVED_IDENTIFIER_FIXUP_6_output += (MatrixEffect_Stage1_c0_c0_h4h4f2(_508, _510) * _4_uKernel_Stage1_c0[6].x);
    _RESERVED_IDENTIFIER_FIXUP_7_coord += _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_6_output *= outputColor_Stage0;
    float4 output_Stage1 = _RESERVED_IDENTIFIER_FIXUP_6_output;
    sk_FragColor = output_Stage1 * outputCoverage_Stage0;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vLocalCoord_Stage0 = stage_input.vLocalCoord_Stage0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
