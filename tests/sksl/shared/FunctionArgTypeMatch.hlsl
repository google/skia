cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _33_colorGreen : packoffset(c0);
    float4 _33_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool takes_void_b()
{
    return true;
}

bool takes_float_bf(float _51)
{
    return true;
}

bool takes_float2_bf2(float2 _54)
{
    return true;
}

bool takes_float3_bf3(float3 _59)
{
    return true;
}

bool takes_float4_bf4(float4 _63)
{
    return true;
}

bool takes_float2x2_bf22(float2x2 _68)
{
    return true;
}

bool takes_float3x3_bf33(float3x3 _73)
{
    return true;
}

bool takes_float4x4_bf44(float4x4 _78)
{
    return true;
}

bool takes_half_bh(float _80)
{
    return true;
}

bool takes_half2_bh2(float2 _82)
{
    return true;
}

bool takes_half3_bh3(float3 _84)
{
    return true;
}

bool takes_half4_bh4(float4 _86)
{
    return true;
}

bool takes_half2x2_bh22(float2x2 _88)
{
    return true;
}

bool takes_half3x3_bh33(float3x3 _90)
{
    return true;
}

bool takes_half4x4_bh44(float4x4 _92)
{
    return true;
}

bool takes_bool_bb(bool _96)
{
    return true;
}

bool takes_bool2_bb2(bool2 _101)
{
    return true;
}

bool takes_bool3_bb3(bool3 _106)
{
    return true;
}

bool takes_bool4_bb4(bool4 _111)
{
    return true;
}

bool takes_int_bi(int _116)
{
    return true;
}

bool takes_int2_bi2(int2 _121)
{
    return true;
}

bool takes_int3_bi3(int3 _126)
{
    return true;
}

bool takes_int4_bi4(int4 _131)
{
    return true;
}

float4 main(float2 _134)
{
    bool _140 = false;
    if (true)
    {
        _140 = takes_void_b();
    }
    else
    {
        _140 = false;
    }
    bool _146 = false;
    if (_140)
    {
        float _144 = 1.0f;
        _146 = takes_float_bf(_144);
    }
    else
    {
        _146 = false;
    }
    bool _153 = false;
    if (_146)
    {
        float2 _151 = 2.0f.xx;
        _153 = takes_float2_bf2(_151);
    }
    else
    {
        _153 = false;
    }
    bool _160 = false;
    if (_153)
    {
        float3 _158 = 3.0f.xxx;
        _160 = takes_float3_bf3(_158);
    }
    else
    {
        _160 = false;
    }
    bool _167 = false;
    if (_160)
    {
        float4 _165 = 4.0f.xxxx;
        _167 = takes_float4_bf4(_165);
    }
    else
    {
        _167 = false;
    }
    bool _175 = false;
    if (_167)
    {
        float2x2 _173 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
        _175 = takes_float2x2_bf22(_173);
    }
    else
    {
        _175 = false;
    }
    bool _184 = false;
    if (_175)
    {
        float3x3 _182 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
        _184 = takes_float3x3_bf33(_182);
    }
    else
    {
        _184 = false;
    }
    bool _194 = false;
    if (_184)
    {
        float4x4 _192 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
        _194 = takes_float4x4_bf44(_192);
    }
    else
    {
        _194 = false;
    }
    bool _199 = false;
    if (_194)
    {
        float _197 = 1.0f;
        _199 = takes_half_bh(_197);
    }
    else
    {
        _199 = false;
    }
    bool _204 = false;
    if (_199)
    {
        float2 _202 = 2.0f.xx;
        _204 = takes_half2_bh2(_202);
    }
    else
    {
        _204 = false;
    }
    bool _209 = false;
    if (_204)
    {
        float3 _207 = 3.0f.xxx;
        _209 = takes_half3_bh3(_207);
    }
    else
    {
        _209 = false;
    }
    bool _214 = false;
    if (_209)
    {
        float4 _212 = 4.0f.xxxx;
        _214 = takes_half4_bh4(_212);
    }
    else
    {
        _214 = false;
    }
    bool _222 = false;
    if (_214)
    {
        float2x2 _220 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
        _222 = takes_half2x2_bh22(_220);
    }
    else
    {
        _222 = false;
    }
    bool _231 = false;
    if (_222)
    {
        float3x3 _229 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
        _231 = takes_half3x3_bh33(_229);
    }
    else
    {
        _231 = false;
    }
    bool _241 = false;
    if (_231)
    {
        float4x4 _239 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
        _241 = takes_half4x4_bh44(_239);
    }
    else
    {
        _241 = false;
    }
    bool _246 = false;
    if (_241)
    {
        bool _244 = true;
        _246 = takes_bool_bb(_244);
    }
    else
    {
        _246 = false;
    }
    bool _252 = false;
    if (_246)
    {
        bool2 _250 = bool2(true, true);
        _252 = takes_bool2_bb2(_250);
    }
    else
    {
        _252 = false;
    }
    bool _258 = false;
    if (_252)
    {
        bool3 _256 = bool3(true, true, true);
        _258 = takes_bool3_bb3(_256);
    }
    else
    {
        _258 = false;
    }
    bool _264 = false;
    if (_258)
    {
        bool4 _262 = bool4(true, true, true, true);
        _264 = takes_bool4_bb4(_262);
    }
    else
    {
        _264 = false;
    }
    bool _270 = false;
    if (_264)
    {
        int _268 = 1;
        _270 = takes_int_bi(_268);
    }
    else
    {
        _270 = false;
    }
    bool _277 = false;
    if (_270)
    {
        int2 _275 = int2(2, 2);
        _277 = takes_int2_bi2(_275);
    }
    else
    {
        _277 = false;
    }
    bool _284 = false;
    if (_277)
    {
        int3 _282 = int3(3, 3, 3);
        _284 = takes_int3_bi3(_282);
    }
    else
    {
        _284 = false;
    }
    bool _291 = false;
    if (_284)
    {
        int4 _289 = int4(4, 4, 4, 4);
        _291 = takes_int4_bi4(_289);
    }
    else
    {
        _291 = false;
    }
    float4 _292 = 0.0f.xxxx;
    if (_291)
    {
        _292 = _33_colorGreen;
    }
    else
    {
        _292 = _33_colorRed;
    }
    return _292;
}

void frag_main()
{
    float2 _43 = 0.0f.xx;
    sk_FragColor = main(_43);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
