cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _31_colorGreen : packoffset(c0);
    float4 _31_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float2 returns_float2_f2()
{
    return 2.0f.xx;
}

float3 returns_float3_f3()
{
    return 3.0f.xxx;
}

float4 returns_float4_f4()
{
    return 4.0f.xxxx;
}

float2x2 returns_float2x2_f22()
{
    return float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
}

float3x3 returns_float3x3_f33()
{
    return float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
}

float4x4 returns_float4x4_f44()
{
    return float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
}

float returns_half_h()
{
    return 1.0f;
}

float2 returns_half2_h2()
{
    return 2.0f.xx;
}

float3 returns_half3_h3()
{
    return 3.0f.xxx;
}

float4 returns_half4_h4()
{
    return 4.0f.xxxx;
}

float2x2 returns_half2x2_h22()
{
    return float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
}

float3x3 returns_half3x3_h33()
{
    return float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
}

float4x4 returns_half4x4_h44()
{
    return float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
}

bool returns_bool_b()
{
    return true;
}

bool2 returns_bool2_b2()
{
    return bool2(true, true);
}

bool3 returns_bool3_b3()
{
    return bool3(true, true, true);
}

bool4 returns_bool4_b4()
{
    return bool4(true, true, true, true);
}

int returns_int_i()
{
    return 1;
}

int2 returns_int2_i2()
{
    return int2(2, 2);
}

int3 returns_int3_i3()
{
    return int3(3, 3, 3);
}

int4 returns_int4_i4()
{
    return int4(4, 4, 4, 4);
}

float4 main(float2 _134)
{
    float x1 = 1.0f;
    float2 x2 = 2.0f.xx;
    float3 x3 = 3.0f.xxx;
    float4 x4 = 4.0f.xxxx;
    float2x2 x5 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
    float3x3 x6 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
    float4x4 x7 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
    float x8 = 1.0f;
    float2 x9 = 2.0f.xx;
    float3 x10 = 3.0f.xxx;
    float4 x11 = 4.0f.xxxx;
    float2x2 x12 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
    float3x3 x13 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
    float4x4 x14 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
    bool x15 = true;
    bool2 x16 = bool2(true, true);
    bool3 x17 = bool3(true, true, true);
    bool4 x18 = bool4(true, true, true, true);
    int x19 = 1;
    int2 x20 = int2(2, 2);
    int3 x21 = int3(3, 3, 3);
    int4 x22 = int4(4, 4, 4, 4);
    bool _205 = false;
    if (x1 == 1.0f)
    {
        float2 _202 = returns_float2_f2();
        _205 = all(bool2(x2.x == _202.x, x2.y == _202.y));
    }
    else
    {
        _205 = false;
    }
    bool _212 = false;
    if (_205)
    {
        float3 _209 = returns_float3_f3();
        _212 = all(bool3(x3.x == _209.x, x3.y == _209.y, x3.z == _209.z));
    }
    else
    {
        _212 = false;
    }
    bool _219 = false;
    if (_212)
    {
        float4 _216 = returns_float4_f4();
        _219 = all(bool4(x4.x == _216.x, x4.y == _216.y, x4.z == _216.z, x4.w == _216.w));
    }
    else
    {
        _219 = false;
    }
    bool _233 = false;
    if (_219)
    {
        float2x2 _223 = returns_float2x2_f22();
        float2 _225 = _223[0];
        float2 _229 = _223[1];
        _233 = all(bool2(x5[0].x == _225.x, x5[0].y == _225.y)) && all(bool2(x5[1].x == _229.x, x5[1].y == _229.y));
    }
    else
    {
        _233 = false;
    }
    bool _252 = false;
    if (_233)
    {
        float3x3 _237 = returns_float3x3_f33();
        float3 _239 = _237[0];
        float3 _243 = _237[1];
        float3 _248 = _237[2];
        _252 = (all(bool3(x6[0].x == _239.x, x6[0].y == _239.y, x6[0].z == _239.z)) && all(bool3(x6[1].x == _243.x, x6[1].y == _243.y, x6[1].z == _243.z))) && all(bool3(x6[2].x == _248.x, x6[2].y == _248.y, x6[2].z == _248.z));
    }
    else
    {
        _252 = false;
    }
    bool _276 = false;
    if (_252)
    {
        float4x4 _256 = returns_float4x4_f44();
        float4 _258 = _256[0];
        float4 _262 = _256[1];
        float4 _267 = _256[2];
        float4 _272 = _256[3];
        _276 = ((all(bool4(x7[0].x == _258.x, x7[0].y == _258.y, x7[0].z == _258.z, x7[0].w == _258.w)) && all(bool4(x7[1].x == _262.x, x7[1].y == _262.y, x7[1].z == _262.z, x7[1].w == _262.w))) && all(bool4(x7[2].x == _267.x, x7[2].y == _267.y, x7[2].z == _267.z, x7[2].w == _267.w))) && all(bool4(x7[3].x == _272.x, x7[3].y == _272.y, x7[3].z == _272.z, x7[3].w == _272.w));
    }
    else
    {
        _276 = false;
    }
    bool _282 = false;
    if (_276)
    {
        _282 = x8 == returns_half_h();
    }
    else
    {
        _282 = false;
    }
    bool _289 = false;
    if (_282)
    {
        float2 _286 = returns_half2_h2();
        _289 = all(bool2(x9.x == _286.x, x9.y == _286.y));
    }
    else
    {
        _289 = false;
    }
    bool _296 = false;
    if (_289)
    {
        float3 _293 = returns_half3_h3();
        _296 = all(bool3(x10.x == _293.x, x10.y == _293.y, x10.z == _293.z));
    }
    else
    {
        _296 = false;
    }
    bool _303 = false;
    if (_296)
    {
        float4 _300 = returns_half4_h4();
        _303 = all(bool4(x11.x == _300.x, x11.y == _300.y, x11.z == _300.z, x11.w == _300.w));
    }
    else
    {
        _303 = false;
    }
    bool _317 = false;
    if (_303)
    {
        float2x2 _307 = returns_half2x2_h22();
        float2 _309 = _307[0];
        float2 _313 = _307[1];
        _317 = all(bool2(x12[0].x == _309.x, x12[0].y == _309.y)) && all(bool2(x12[1].x == _313.x, x12[1].y == _313.y));
    }
    else
    {
        _317 = false;
    }
    bool _336 = false;
    if (_317)
    {
        float3x3 _321 = returns_half3x3_h33();
        float3 _323 = _321[0];
        float3 _327 = _321[1];
        float3 _332 = _321[2];
        _336 = (all(bool3(x13[0].x == _323.x, x13[0].y == _323.y, x13[0].z == _323.z)) && all(bool3(x13[1].x == _327.x, x13[1].y == _327.y, x13[1].z == _327.z))) && all(bool3(x13[2].x == _332.x, x13[2].y == _332.y, x13[2].z == _332.z));
    }
    else
    {
        _336 = false;
    }
    bool _360 = false;
    if (_336)
    {
        float4x4 _340 = returns_half4x4_h44();
        float4 _342 = _340[0];
        float4 _346 = _340[1];
        float4 _351 = _340[2];
        float4 _356 = _340[3];
        _360 = ((all(bool4(x14[0].x == _342.x, x14[0].y == _342.y, x14[0].z == _342.z, x14[0].w == _342.w)) && all(bool4(x14[1].x == _346.x, x14[1].y == _346.y, x14[1].z == _346.z, x14[1].w == _346.w))) && all(bool4(x14[2].x == _351.x, x14[2].y == _351.y, x14[2].z == _351.z, x14[2].w == _351.w))) && all(bool4(x14[3].x == _356.x, x14[3].y == _356.y, x14[3].z == _356.z, x14[3].w == _356.w));
    }
    else
    {
        _360 = false;
    }
    bool _366 = false;
    if (_360)
    {
        _366 = x15 == returns_bool_b();
    }
    else
    {
        _366 = false;
    }
    bool _373 = false;
    if (_366)
    {
        bool2 _370 = returns_bool2_b2();
        _373 = all(bool2(x16.x == _370.x, x16.y == _370.y));
    }
    else
    {
        _373 = false;
    }
    bool _380 = false;
    if (_373)
    {
        bool3 _377 = returns_bool3_b3();
        _380 = all(bool3(x17.x == _377.x, x17.y == _377.y, x17.z == _377.z));
    }
    else
    {
        _380 = false;
    }
    bool _387 = false;
    if (_380)
    {
        bool4 _384 = returns_bool4_b4();
        _387 = all(bool4(x18.x == _384.x, x18.y == _384.y, x18.z == _384.z, x18.w == _384.w));
    }
    else
    {
        _387 = false;
    }
    bool _393 = false;
    if (_387)
    {
        _393 = x19 == returns_int_i();
    }
    else
    {
        _393 = false;
    }
    bool _400 = false;
    if (_393)
    {
        int2 _397 = returns_int2_i2();
        _400 = all(bool2(x20.x == _397.x, x20.y == _397.y));
    }
    else
    {
        _400 = false;
    }
    bool _407 = false;
    if (_400)
    {
        int3 _404 = returns_int3_i3();
        _407 = all(bool3(x21.x == _404.x, x21.y == _404.y, x21.z == _404.z));
    }
    else
    {
        _407 = false;
    }
    bool _414 = false;
    if (_407)
    {
        int4 _411 = returns_int4_i4();
        _414 = all(bool4(x22.x == _411.x, x22.y == _411.y, x22.z == _411.z, x22.w == _411.w));
    }
    else
    {
        _414 = false;
    }
    float4 _415 = 0.0f.xxxx;
    if (_414)
    {
        _415 = _31_colorGreen;
    }
    else
    {
        _415 = _31_colorRed;
    }
    return _415;
}

void frag_main()
{
    float2 _41 = 0.0f.xx;
    sk_FragColor = main(_41);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
