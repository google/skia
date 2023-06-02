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
    bool _219 = false;
    if (_214)
    {
        float2x2 _217 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
        _219 = takes_half2x2_bh22(_217);
    }
    else
    {
        _219 = false;
    }
    bool _224 = false;
    if (_219)
    {
        float3x3 _222 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
        _224 = takes_half3x3_bh33(_222);
    }
    else
    {
        _224 = false;
    }
    bool _229 = false;
    if (_224)
    {
        float4x4 _227 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
        _229 = takes_half4x4_bh44(_227);
    }
    else
    {
        _229 = false;
    }
    bool _234 = false;
    if (_229)
    {
        bool _232 = true;
        _234 = takes_bool_bb(_232);
    }
    else
    {
        _234 = false;
    }
    bool _240 = false;
    if (_234)
    {
        bool2 _238 = bool2(true, true);
        _240 = takes_bool2_bb2(_238);
    }
    else
    {
        _240 = false;
    }
    bool _246 = false;
    if (_240)
    {
        bool3 _244 = bool3(true, true, true);
        _246 = takes_bool3_bb3(_244);
    }
    else
    {
        _246 = false;
    }
    bool _252 = false;
    if (_246)
    {
        bool4 _250 = bool4(true, true, true, true);
        _252 = takes_bool4_bb4(_250);
    }
    else
    {
        _252 = false;
    }
    bool _258 = false;
    if (_252)
    {
        int _256 = 1;
        _258 = takes_int_bi(_256);
    }
    else
    {
        _258 = false;
    }
    bool _265 = false;
    if (_258)
    {
        int2 _263 = int2(2, 2);
        _265 = takes_int2_bi2(_263);
    }
    else
    {
        _265 = false;
    }
    bool _272 = false;
    if (_265)
    {
        int3 _270 = int3(3, 3, 3);
        _272 = takes_int3_bi3(_270);
    }
    else
    {
        _272 = false;
    }
    bool _279 = false;
    if (_272)
    {
        int4 _277 = int4(4, 4, 4, 4);
        _279 = takes_int4_bi4(_277);
    }
    else
    {
        _279 = false;
    }
    float4 _280 = 0.0f.xxxx;
    if (_279)
    {
        _280 = _33_colorGreen;
    }
    else
    {
        _280 = _33_colorRed;
    }
    return _280;
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
