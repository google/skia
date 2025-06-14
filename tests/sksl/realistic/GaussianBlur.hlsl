cbuffer uniformBuffer : register(b0, space0)
{
    float4 _8_sk_RTAdjust : packoffset(c0);
    float2 _8_uIncrement_Stage1_c0 : packoffset(c1);
    float4 _8_uKernel_Stage1_c0[7] : packoffset(c2);
    row_major float3x3 _8_umatrix_Stage1_c0_c0 : packoffset(c9);
    float4 _8_uborder_Stage1_c0_c0_c0 : packoffset(c12);
    float4 _8_usubset_Stage1_c0_c0_c0 : packoffset(c13);
    float4 _8_unorm_Stage1_c0_c0_c0 : packoffset(c14);
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
    float2 _43 = mul(float3(_30, 1.0f), _8_umatrix_Stage1_c0_c0).xy;
    float2 _RESERVED_IDENTIFIER_FIXUP_1_inCoord = _43;
    float2 _49 = _43 * _8_unorm_Stage1_c0_c0_c0.xy;
    _RESERVED_IDENTIFIER_FIXUP_1_inCoord = _49;
    float2 _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord = 0.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord.x = _49.x;
    _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord.y = _RESERVED_IDENTIFIER_FIXUP_1_inCoord.y;
    float2 _RESERVED_IDENTIFIER_FIXUP_3_clampedCoord = _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord;
    float4 _RESERVED_IDENTIFIER_FIXUP_4_textureColor = uTextureSampler_0_Stage1.Sample(_uTextureSampler_0_Stage1_sampler, _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord * _8_unorm_Stage1_c0_c0_c0.zw);
    float _75 = floor(_RESERVED_IDENTIFIER_FIXUP_1_inCoord.x + 0.001000000047497451305389404296875f) + 0.5f;
    float _RESERVED_IDENTIFIER_FIXUP_5_snappedX = _75;
    bool _89 = false;
    if (_75 < _8_usubset_Stage1_c0_c0_c0.x)
    {
        _89 = true;
    }
    else
    {
        _89 = _75 > _8_usubset_Stage1_c0_c0_c0.z;
    }
    if (_89)
    {
        _RESERVED_IDENTIFIER_FIXUP_4_textureColor = _8_uborder_Stage1_c0_c0_c0;
    }
    return _RESERVED_IDENTIFIER_FIXUP_4_textureColor;
}

void frag_main()
{
    float4 outputColor_Stage0 = 1.0f.xxxx;
    float4 outputCoverage_Stage0 = 1.0f.xxxx;
    float4 _RESERVED_IDENTIFIER_FIXUP_6_output = 0.0f.xxxx;
    float2 _112 = vLocalCoord_Stage0 - (_8_uIncrement_Stage1_c0 * 12.0f);
    float2 _RESERVED_IDENTIFIER_FIXUP_7_coord = _112;
    float2 _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = 0.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _112;
    float4 _115 = 1.0f.xxxx;
    float2 _116 = _112;
    float4 _123 = 0.0f.xxxx + (MatrixEffect_Stage1_c0_c0_h4h4f2(_115, _116) * _8_uKernel_Stage1_c0[0].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _123;
    float2 _126 = _112 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _126;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _126;
    float4 _127 = 1.0f.xxxx;
    float2 _128 = _126;
    float4 _134 = _123 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_127, _128) * _8_uKernel_Stage1_c0[0].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _134;
    float2 _137 = _126 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _137;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _137;
    float4 _138 = 1.0f.xxxx;
    float2 _139 = _137;
    float4 _145 = _134 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_138, _139) * _8_uKernel_Stage1_c0[0].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _145;
    float2 _148 = _137 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _148;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _148;
    float4 _149 = 1.0f.xxxx;
    float2 _150 = _148;
    float4 _156 = _145 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_149, _150) * _8_uKernel_Stage1_c0[0].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _156;
    float2 _159 = _148 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _159;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _159;
    float4 _160 = 1.0f.xxxx;
    float2 _161 = _159;
    float4 _167 = _156 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_160, _161) * _8_uKernel_Stage1_c0[1].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _167;
    float2 _170 = _159 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _170;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _170;
    float4 _171 = 1.0f.xxxx;
    float2 _172 = _170;
    float4 _178 = _167 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_171, _172) * _8_uKernel_Stage1_c0[1].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _178;
    float2 _181 = _170 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _181;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _181;
    float4 _182 = 1.0f.xxxx;
    float2 _183 = _181;
    float4 _189 = _178 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_182, _183) * _8_uKernel_Stage1_c0[1].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _189;
    float2 _192 = _181 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _192;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _192;
    float4 _193 = 1.0f.xxxx;
    float2 _194 = _192;
    float4 _200 = _189 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_193, _194) * _8_uKernel_Stage1_c0[1].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _200;
    float2 _203 = _192 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _203;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _203;
    float4 _204 = 1.0f.xxxx;
    float2 _205 = _203;
    float4 _211 = _200 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_204, _205) * _8_uKernel_Stage1_c0[2].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _211;
    float2 _214 = _203 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _214;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _214;
    float4 _215 = 1.0f.xxxx;
    float2 _216 = _214;
    float4 _222 = _211 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_215, _216) * _8_uKernel_Stage1_c0[2].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _222;
    float2 _225 = _214 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _225;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _225;
    float4 _226 = 1.0f.xxxx;
    float2 _227 = _225;
    float4 _233 = _222 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_226, _227) * _8_uKernel_Stage1_c0[2].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _233;
    float2 _236 = _225 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _236;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _236;
    float4 _237 = 1.0f.xxxx;
    float2 _238 = _236;
    float4 _244 = _233 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_237, _238) * _8_uKernel_Stage1_c0[2].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _244;
    float2 _247 = _236 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _247;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _247;
    float4 _248 = 1.0f.xxxx;
    float2 _249 = _247;
    float4 _255 = _244 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_248, _249) * _8_uKernel_Stage1_c0[3].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _255;
    float2 _258 = _247 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _258;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _258;
    float4 _259 = 1.0f.xxxx;
    float2 _260 = _258;
    float4 _266 = _255 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_259, _260) * _8_uKernel_Stage1_c0[3].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _266;
    float2 _269 = _258 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _269;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _269;
    float4 _270 = 1.0f.xxxx;
    float2 _271 = _269;
    float4 _277 = _266 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_270, _271) * _8_uKernel_Stage1_c0[3].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _277;
    float2 _280 = _269 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _280;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _280;
    float4 _281 = 1.0f.xxxx;
    float2 _282 = _280;
    float4 _288 = _277 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_281, _282) * _8_uKernel_Stage1_c0[3].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _288;
    float2 _291 = _280 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _291;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _291;
    float4 _292 = 1.0f.xxxx;
    float2 _293 = _291;
    float4 _299 = _288 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_292, _293) * _8_uKernel_Stage1_c0[4].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _299;
    float2 _302 = _291 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _302;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _302;
    float4 _303 = 1.0f.xxxx;
    float2 _304 = _302;
    float4 _310 = _299 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_303, _304) * _8_uKernel_Stage1_c0[4].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _310;
    float2 _313 = _302 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _313;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _313;
    float4 _314 = 1.0f.xxxx;
    float2 _315 = _313;
    float4 _321 = _310 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_314, _315) * _8_uKernel_Stage1_c0[4].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _321;
    float2 _324 = _313 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _324;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _324;
    float4 _325 = 1.0f.xxxx;
    float2 _326 = _324;
    float4 _332 = _321 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_325, _326) * _8_uKernel_Stage1_c0[4].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _332;
    float2 _335 = _324 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _335;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _335;
    float4 _336 = 1.0f.xxxx;
    float2 _337 = _335;
    float4 _343 = _332 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_336, _337) * _8_uKernel_Stage1_c0[5].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _343;
    float2 _346 = _335 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _346;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _346;
    float4 _347 = 1.0f.xxxx;
    float2 _348 = _346;
    float4 _354 = _343 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_347, _348) * _8_uKernel_Stage1_c0[5].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _354;
    float2 _357 = _346 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _357;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _357;
    float4 _358 = 1.0f.xxxx;
    float2 _359 = _357;
    float4 _365 = _354 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_358, _359) * _8_uKernel_Stage1_c0[5].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _365;
    float2 _368 = _357 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _368;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _368;
    float4 _369 = 1.0f.xxxx;
    float2 _370 = _368;
    float4 _376 = _365 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_369, _370) * _8_uKernel_Stage1_c0[5].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _376;
    float2 _379 = _368 + _8_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _379;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _379;
    float4 _380 = 1.0f.xxxx;
    float2 _381 = _379;
    float4 _387 = _376 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_380, _381) * _8_uKernel_Stage1_c0[6].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _387;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _379 + _8_uIncrement_Stage1_c0;
    float4 _391 = _387 * 1.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_6_output = _391;
    float4 output_Stage1 = _391;
    sk_FragColor = _391 * 1.0f.xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vLocalCoord_Stage0 = stage_input.vLocalCoord_Stage0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
