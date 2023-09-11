cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _30_colorGreen : packoffset(c0);
    float4 _30_colorRed : packoffset(c1);
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

bool takes_float_bf(float _49)
{
    return true;
}

bool takes_float2_bf2(float2 _52)
{
    return true;
}

bool takes_float3_bf3(float3 _57)
{
    return true;
}

bool takes_float4_bf4(float4 _61)
{
    return true;
}

bool takes_float2x2_bf22(float2x2 _66)
{
    return true;
}

bool takes_float3x3_bf33(float3x3 _71)
{
    return true;
}

bool takes_float4x4_bf44(float4x4 _76)
{
    return true;
}

bool takes_half_bh(float _78)
{
    return true;
}

bool takes_half2_bh2(float2 _80)
{
    return true;
}

bool takes_half3_bh3(float3 _82)
{
    return true;
}

bool takes_half4_bh4(float4 _84)
{
    return true;
}

bool takes_half2x2_bh22(float2x2 _86)
{
    return true;
}

bool takes_half3x3_bh33(float3x3 _88)
{
    return true;
}

bool takes_half4x4_bh44(float4x4 _90)
{
    return true;
}

bool takes_bool_bb(bool _94)
{
    return true;
}

bool takes_bool2_bb2(bool2 _99)
{
    return true;
}

bool takes_bool3_bb3(bool3 _104)
{
    return true;
}

bool takes_bool4_bb4(bool4 _109)
{
    return true;
}

bool takes_int_bi(int _114)
{
    return true;
}

bool takes_int2_bi2(int2 _119)
{
    return true;
}

bool takes_int3_bi3(int3 _124)
{
    return true;
}

bool takes_int4_bi4(int4 _129)
{
    return true;
}

float4 main(float2 _132)
{
    bool _138 = false;
    if (true)
    {
        _138 = takes_void_b();
    }
    else
    {
        _138 = false;
    }
    bool _144 = false;
    if (_138)
    {
        float _142 = 1.0f;
        _144 = takes_float_bf(_142);
    }
    else
    {
        _144 = false;
    }
    bool _151 = false;
    if (_144)
    {
        float2 _149 = 2.0f.xx;
        _151 = takes_float2_bf2(_149);
    }
    else
    {
        _151 = false;
    }
    bool _158 = false;
    if (_151)
    {
        float3 _156 = 3.0f.xxx;
        _158 = takes_float3_bf3(_156);
    }
    else
    {
        _158 = false;
    }
    bool _165 = false;
    if (_158)
    {
        float4 _163 = 4.0f.xxxx;
        _165 = takes_float4_bf4(_163);
    }
    else
    {
        _165 = false;
    }
    bool _173 = false;
    if (_165)
    {
        float2x2 _171 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
        _173 = takes_float2x2_bf22(_171);
    }
    else
    {
        _173 = false;
    }
    bool _182 = false;
    if (_173)
    {
        float3x3 _180 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
        _182 = takes_float3x3_bf33(_180);
    }
    else
    {
        _182 = false;
    }
    bool _192 = false;
    if (_182)
    {
        float4x4 _190 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
        _192 = takes_float4x4_bf44(_190);
    }
    else
    {
        _192 = false;
    }
    bool _197 = false;
    if (_192)
    {
        float _195 = 1.0f;
        _197 = takes_half_bh(_195);
    }
    else
    {
        _197 = false;
    }
    bool _202 = false;
    if (_197)
    {
        float2 _200 = 2.0f.xx;
        _202 = takes_half2_bh2(_200);
    }
    else
    {
        _202 = false;
    }
    bool _207 = false;
    if (_202)
    {
        float3 _205 = 3.0f.xxx;
        _207 = takes_half3_bh3(_205);
    }
    else
    {
        _207 = false;
    }
    bool _212 = false;
    if (_207)
    {
        float4 _210 = 4.0f.xxxx;
        _212 = takes_half4_bh4(_210);
    }
    else
    {
        _212 = false;
    }
    bool _217 = false;
    if (_212)
    {
        float2x2 _215 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
        _217 = takes_half2x2_bh22(_215);
    }
    else
    {
        _217 = false;
    }
    bool _222 = false;
    if (_217)
    {
        float3x3 _220 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
        _222 = takes_half3x3_bh33(_220);
    }
    else
    {
        _222 = false;
    }
    bool _227 = false;
    if (_222)
    {
        float4x4 _225 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
        _227 = takes_half4x4_bh44(_225);
    }
    else
    {
        _227 = false;
    }
    bool _232 = false;
    if (_227)
    {
        bool _230 = true;
        _232 = takes_bool_bb(_230);
    }
    else
    {
        _232 = false;
    }
    bool _238 = false;
    if (_232)
    {
        bool2 _236 = bool2(true, true);
        _238 = takes_bool2_bb2(_236);
    }
    else
    {
        _238 = false;
    }
    bool _244 = false;
    if (_238)
    {
        bool3 _242 = bool3(true, true, true);
        _244 = takes_bool3_bb3(_242);
    }
    else
    {
        _244 = false;
    }
    bool _250 = false;
    if (_244)
    {
        bool4 _248 = bool4(true, true, true, true);
        _250 = takes_bool4_bb4(_248);
    }
    else
    {
        _250 = false;
    }
    bool _256 = false;
    if (_250)
    {
        int _254 = 1;
        _256 = takes_int_bi(_254);
    }
    else
    {
        _256 = false;
    }
    bool _263 = false;
    if (_256)
    {
        int2 _261 = int2(2, 2);
        _263 = takes_int2_bi2(_261);
    }
    else
    {
        _263 = false;
    }
    bool _270 = false;
    if (_263)
    {
        int3 _268 = int3(3, 3, 3);
        _270 = takes_int3_bi3(_268);
    }
    else
    {
        _270 = false;
    }
    bool _277 = false;
    if (_270)
    {
        int4 _275 = int4(4, 4, 4, 4);
        _277 = takes_int4_bi4(_275);
    }
    else
    {
        _277 = false;
    }
    float4 _278 = 0.0f.xxxx;
    if (_277)
    {
        _278 = _30_colorGreen;
    }
    else
    {
        _278 = _30_colorRed;
    }
    return _278;
}

void frag_main()
{
    float2 _40 = 0.0f.xx;
    sk_FragColor = main(_40);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
