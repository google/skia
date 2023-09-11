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

float4 MatrixEffect_Stage1_c0_c0_h4h4f2(float4 _26, float2 _27)
{
    float2 _40 = mul(float3(_27, 1.0f), _4_umatrix_Stage1_c0_c0).xy;
    float2 _RESERVED_IDENTIFIER_FIXUP_1_inCoord = _40;
    float2 _46 = _40 * _4_unorm_Stage1_c0_c0_c0.xy;
    _RESERVED_IDENTIFIER_FIXUP_1_inCoord = _46;
    float2 _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord = 0.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord.x = _46.x;
    _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord.y = _RESERVED_IDENTIFIER_FIXUP_1_inCoord.y;
    float2 _RESERVED_IDENTIFIER_FIXUP_3_clampedCoord = _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord;
    float4 _RESERVED_IDENTIFIER_FIXUP_4_textureColor = uTextureSampler_0_Stage1.Sample(_uTextureSampler_0_Stage1_sampler, _RESERVED_IDENTIFIER_FIXUP_2_subsetCoord * _4_unorm_Stage1_c0_c0_c0.zw);
    float _72 = floor(_RESERVED_IDENTIFIER_FIXUP_1_inCoord.x + 0.001000000047497451305389404296875f) + 0.5f;
    float _RESERVED_IDENTIFIER_FIXUP_5_snappedX = _72;
    bool _86 = false;
    if (_72 < _4_usubset_Stage1_c0_c0_c0.x)
    {
        _86 = true;
    }
    else
    {
        _86 = _72 > _4_usubset_Stage1_c0_c0_c0.z;
    }
    if (_86)
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
    float2 _109 = vLocalCoord_Stage0 - (_4_uIncrement_Stage1_c0 * 12.0f);
    float2 _RESERVED_IDENTIFIER_FIXUP_7_coord = _109;
    float2 _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = 0.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _109;
    float4 _112 = 1.0f.xxxx;
    float2 _113 = _109;
    float4 _120 = 0.0f.xxxx + (MatrixEffect_Stage1_c0_c0_h4h4f2(_112, _113) * _4_uKernel_Stage1_c0[0].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _120;
    float2 _123 = _109 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _123;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _123;
    float4 _124 = 1.0f.xxxx;
    float2 _125 = _123;
    float4 _131 = _120 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_124, _125) * _4_uKernel_Stage1_c0[0].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _131;
    float2 _134 = _123 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _134;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _134;
    float4 _135 = 1.0f.xxxx;
    float2 _136 = _134;
    float4 _142 = _131 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_135, _136) * _4_uKernel_Stage1_c0[0].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _142;
    float2 _145 = _134 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _145;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _145;
    float4 _146 = 1.0f.xxxx;
    float2 _147 = _145;
    float4 _153 = _142 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_146, _147) * _4_uKernel_Stage1_c0[0].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _153;
    float2 _156 = _145 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _156;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _156;
    float4 _157 = 1.0f.xxxx;
    float2 _158 = _156;
    float4 _164 = _153 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_157, _158) * _4_uKernel_Stage1_c0[1].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _164;
    float2 _167 = _156 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _167;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _167;
    float4 _168 = 1.0f.xxxx;
    float2 _169 = _167;
    float4 _175 = _164 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_168, _169) * _4_uKernel_Stage1_c0[1].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _175;
    float2 _178 = _167 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _178;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _178;
    float4 _179 = 1.0f.xxxx;
    float2 _180 = _178;
    float4 _186 = _175 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_179, _180) * _4_uKernel_Stage1_c0[1].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _186;
    float2 _189 = _178 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _189;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _189;
    float4 _190 = 1.0f.xxxx;
    float2 _191 = _189;
    float4 _197 = _186 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_190, _191) * _4_uKernel_Stage1_c0[1].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _197;
    float2 _200 = _189 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _200;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _200;
    float4 _201 = 1.0f.xxxx;
    float2 _202 = _200;
    float4 _208 = _197 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_201, _202) * _4_uKernel_Stage1_c0[2].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _208;
    float2 _211 = _200 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _211;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _211;
    float4 _212 = 1.0f.xxxx;
    float2 _213 = _211;
    float4 _219 = _208 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_212, _213) * _4_uKernel_Stage1_c0[2].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _219;
    float2 _222 = _211 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _222;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _222;
    float4 _223 = 1.0f.xxxx;
    float2 _224 = _222;
    float4 _230 = _219 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_223, _224) * _4_uKernel_Stage1_c0[2].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _230;
    float2 _233 = _222 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _233;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _233;
    float4 _234 = 1.0f.xxxx;
    float2 _235 = _233;
    float4 _241 = _230 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_234, _235) * _4_uKernel_Stage1_c0[2].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _241;
    float2 _244 = _233 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _244;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _244;
    float4 _245 = 1.0f.xxxx;
    float2 _246 = _244;
    float4 _252 = _241 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_245, _246) * _4_uKernel_Stage1_c0[3].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _252;
    float2 _255 = _244 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _255;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _255;
    float4 _256 = 1.0f.xxxx;
    float2 _257 = _255;
    float4 _263 = _252 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_256, _257) * _4_uKernel_Stage1_c0[3].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _263;
    float2 _266 = _255 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _266;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _266;
    float4 _267 = 1.0f.xxxx;
    float2 _268 = _266;
    float4 _274 = _263 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_267, _268) * _4_uKernel_Stage1_c0[3].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _274;
    float2 _277 = _266 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _277;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _277;
    float4 _278 = 1.0f.xxxx;
    float2 _279 = _277;
    float4 _285 = _274 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_278, _279) * _4_uKernel_Stage1_c0[3].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _285;
    float2 _288 = _277 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _288;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _288;
    float4 _289 = 1.0f.xxxx;
    float2 _290 = _288;
    float4 _296 = _285 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_289, _290) * _4_uKernel_Stage1_c0[4].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _296;
    float2 _299 = _288 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _299;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _299;
    float4 _300 = 1.0f.xxxx;
    float2 _301 = _299;
    float4 _307 = _296 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_300, _301) * _4_uKernel_Stage1_c0[4].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _307;
    float2 _310 = _299 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _310;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _310;
    float4 _311 = 1.0f.xxxx;
    float2 _312 = _310;
    float4 _318 = _307 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_311, _312) * _4_uKernel_Stage1_c0[4].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _318;
    float2 _321 = _310 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _321;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _321;
    float4 _322 = 1.0f.xxxx;
    float2 _323 = _321;
    float4 _329 = _318 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_322, _323) * _4_uKernel_Stage1_c0[4].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _329;
    float2 _332 = _321 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _332;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _332;
    float4 _333 = 1.0f.xxxx;
    float2 _334 = _332;
    float4 _340 = _329 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_333, _334) * _4_uKernel_Stage1_c0[5].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _340;
    float2 _343 = _332 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _343;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _343;
    float4 _344 = 1.0f.xxxx;
    float2 _345 = _343;
    float4 _351 = _340 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_344, _345) * _4_uKernel_Stage1_c0[5].y);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _351;
    float2 _354 = _343 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _354;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _354;
    float4 _355 = 1.0f.xxxx;
    float2 _356 = _354;
    float4 _362 = _351 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_355, _356) * _4_uKernel_Stage1_c0[5].z);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _362;
    float2 _365 = _354 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _365;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _365;
    float4 _366 = 1.0f.xxxx;
    float2 _367 = _365;
    float4 _373 = _362 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_366, _367) * _4_uKernel_Stage1_c0[5].w);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _373;
    float2 _376 = _365 + _4_uIncrement_Stage1_c0;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _376;
    _RESERVED_IDENTIFIER_FIXUP_8_coordSampled = _376;
    float4 _377 = 1.0f.xxxx;
    float2 _378 = _376;
    float4 _384 = _373 + (MatrixEffect_Stage1_c0_c0_h4h4f2(_377, _378) * _4_uKernel_Stage1_c0[6].x);
    _RESERVED_IDENTIFIER_FIXUP_6_output = _384;
    _RESERVED_IDENTIFIER_FIXUP_7_coord = _376 + _4_uIncrement_Stage1_c0;
    float4 _388 = _384 * 1.0f.xxxx;
    _RESERVED_IDENTIFIER_FIXUP_6_output = _388;
    float4 output_Stage1 = _388;
    sk_FragColor = _388 * 1.0f.xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vLocalCoord_Stage0 = stage_input.vLocalCoord_Stage0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
