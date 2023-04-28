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
    float2 _43 = mul(float3(_30, 1.0f), _4_umatrix_Stage1_c0_c0).xy;
    float2 _RESERVED_IDENTIFIER_FIXUP_1_inCoord = _43;
    float2 _49 = _43 * _4_unorm_Stage1_c0_c0_c0.xy;
    _RESERVED_IDENTIFIER_FIXUP_1_inCoord = _49;
    float2 _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord = 0.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord.x = _49.x;
    _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord.y = _RESERVED_IDENTIFIER_FIXUP_1_inCoord.y;
    float2 _RESERVED_IDENTIFIER_FIXUP_3_clampedCoord = _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord;
    float4 _RESERVED_IDENTIFIER_FIXUP_4_textureColor = uTextureSampler_0_Stage1.Sample(_uTextureSampler_0_Stage1_sampler, _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord * _4_unorm_Stage1_c0_c0_c0.zw);
    float _75 = floor(_RESERVED_IDENTIFIER_FIXUP_1_inCoord.x + 0.001000000047497451305389404296875f) + 0.5f;
    float _RESERVED_IDENTIFIER_FIXUP_5_snappedX = _75;
    bool _88 = false;
    if (_75 < _4_usubset_Stage1_c0_c0_c0.x)
    {
        _88 = true;
    }
    else
    {
        _88 = _75 > _4_usubset_Stage1_c0_c0_c0.z;
    }
    if (_88)
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
    float2 _111 = vLocalCoord_Stage0 - (_4_uIncrement_Stage1_c0 * 12.0f);
    float2 _RESERVED_IDENTIFIER_FIXUP_7_coord = _111;
    float2 _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = 0.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _111;
    float4 _114 = 1.0f.xxxx;
    float2 _115 = _111;
    float4 _122 = 0.0f.xxxx + (MatrixEffect_Stage1_c0_c0_h4h4f2(_114, _115) * _4_uKernel_Stage1_c0[0].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _122;
    float2 _125 = _111 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _125;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _125;
    float4 _126 = 1.0f.xxxx;
    float2 _127 = _125;
    float4 _133 = _122 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_126, _127) * _4_uKernel_Stage1_c0[0].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _133;
    float2 _136 = _125 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _136;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _136;
    float4 _137 = 1.0f.xxxx;
    float2 _138 = _136;
    float4 _144 = _133 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_137, _138) * _4_uKernel_Stage1_c0[0].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _144;
    float2 _147 = _136 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _147;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _147;
    float4 _148 = 1.0f.xxxx;
    float2 _149 = _147;
    float4 _155 = _144 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_148, _149) * _4_uKernel_Stage1_c0[0].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _155;
    float2 _158 = _147 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _158;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _158;
    float4 _159 = 1.0f.xxxx;
    float2 _160 = _158;
    float4 _166 = _155 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_159, _160) * _4_uKernel_Stage1_c0[1].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _166;
    float2 _169 = _158 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _169;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _169;
    float4 _170 = 1.0f.xxxx;
    float2 _171 = _169;
    float4 _177 = _166 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_170, _171) * _4_uKernel_Stage1_c0[1].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _177;
    float2 _180 = _169 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _180;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _180;
    float4 _181 = 1.0f.xxxx;
    float2 _182 = _180;
    float4 _188 = _177 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_181, _182) * _4_uKernel_Stage1_c0[1].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _188;
    float2 _191 = _180 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _191;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _191;
    float4 _192 = 1.0f.xxxx;
    float2 _193 = _191;
    float4 _199 = _188 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_192, _193) * _4_uKernel_Stage1_c0[1].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _199;
    float2 _202 = _191 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _202;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _202;
    float4 _203 = 1.0f.xxxx;
    float2 _204 = _202;
    float4 _210 = _199 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_203, _204) * _4_uKernel_Stage1_c0[2].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _210;
    float2 _213 = _202 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _213;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _213;
    float4 _214 = 1.0f.xxxx;
    float2 _215 = _213;
    float4 _221 = _210 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_214, _215) * _4_uKernel_Stage1_c0[2].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _221;
    float2 _224 = _213 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _224;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _224;
    float4 _225 = 1.0f.xxxx;
    float2 _226 = _224;
    float4 _232 = _221 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_225, _226) * _4_uKernel_Stage1_c0[2].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _232;
    float2 _235 = _224 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _235;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _235;
    float4 _236 = 1.0f.xxxx;
    float2 _237 = _235;
    float4 _243 = _232 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_236, _237) * _4_uKernel_Stage1_c0[2].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _243;
    float2 _246 = _235 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _246;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _246;
    float4 _247 = 1.0f.xxxx;
    float2 _248 = _246;
    float4 _254 = _243 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_247, _248) * _4_uKernel_Stage1_c0[3].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _254;
    float2 _257 = _246 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _257;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _257;
    float4 _258 = 1.0f.xxxx;
    float2 _259 = _257;
    float4 _265 = _254 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_258, _259) * _4_uKernel_Stage1_c0[3].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _265;
    float2 _268 = _257 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _268;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _268;
    float4 _269 = 1.0f.xxxx;
    float2 _270 = _268;
    float4 _276 = _265 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_269, _270) * _4_uKernel_Stage1_c0[3].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _276;
    float2 _279 = _268 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _279;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _279;
    float4 _280 = 1.0f.xxxx;
    float2 _281 = _279;
    float4 _287 = _276 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_280, _281) * _4_uKernel_Stage1_c0[3].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _287;
    float2 _290 = _279 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _290;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _290;
    float4 _291 = 1.0f.xxxx;
    float2 _292 = _290;
    float4 _298 = _287 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_291, _292) * _4_uKernel_Stage1_c0[4].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _298;
    float2 _301 = _290 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _301;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _301;
    float4 _302 = 1.0f.xxxx;
    float2 _303 = _301;
    float4 _309 = _298 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_302, _303) * _4_uKernel_Stage1_c0[4].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _309;
    float2 _312 = _301 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _312;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _312;
    float4 _313 = 1.0f.xxxx;
    float2 _314 = _312;
    float4 _320 = _309 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_313, _314) * _4_uKernel_Stage1_c0[4].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _320;
    float2 _323 = _312 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _323;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _323;
    float4 _324 = 1.0f.xxxx;
    float2 _325 = _323;
    float4 _331 = _320 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_324, _325) * _4_uKernel_Stage1_c0[4].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _331;
    float2 _334 = _323 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _334;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _334;
    float4 _335 = 1.0f.xxxx;
    float2 _336 = _334;
    float4 _342 = _331 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_335, _336) * _4_uKernel_Stage1_c0[5].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _342;
    float2 _345 = _334 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _345;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _345;
    float4 _346 = 1.0f.xxxx;
    float2 _347 = _345;
    float4 _353 = _342 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_346, _347) * _4_uKernel_Stage1_c0[5].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _353;
    float2 _356 = _345 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _356;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _356;
    float4 _357 = 1.0f.xxxx;
    float2 _358 = _356;
    float4 _364 = _353 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_357, _358) * _4_uKernel_Stage1_c0[5].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _364;
    float2 _367 = _356 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _367;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _367;
    float4 _368 = 1.0f.xxxx;
    float2 _369 = _367;
    float4 _375 = _364 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_368, _369) * _4_uKernel_Stage1_c0[5].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _375;
    float2 _378 = _367 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _378;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _378;
    float4 _379 = 1.0f.xxxx;
    float2 _380 = _378;
    float4 _386 = _375 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_379, _380) * _4_uKernel_Stage1_c0[6].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _386;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _378 + _4_uIncrement_Stage1_c0;
    float4 _390 = _386 * 1.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_6_output = _390;
    float4 output_Stage1 = _390;
    sk_FragColor = _390 * 1.0f.xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vLocalCoord_Stage0 = stage_input.vLocalCoord_Stage0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
