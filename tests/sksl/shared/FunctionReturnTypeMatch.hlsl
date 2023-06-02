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

float4 main(float2 _122)
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
    bool _166 = false;
    if (true)
    {
        float2 _163 = returns_float2_f2();
        _166 = all(bool2(2.0f.xx.x == _163.x, 2.0f.xx.y == _163.y));
    }
    else
    {
        _166 = false;
    }
    bool _172 = false;
    if (_166)
    {
        float3 _169 = returns_float3_f3();
        _172 = all(bool3(3.0f.xxx.x == _169.x, 3.0f.xxx.y == _169.y, 3.0f.xxx.z == _169.z));
    }
    else
    {
        _172 = false;
    }
    bool _178 = false;
    if (_172)
    {
        float4 _175 = returns_float4_f4();
        _178 = all(bool4(4.0f.xxxx.x == _175.x, 4.0f.xxxx.y == _175.y, 4.0f.xxxx.z == _175.z, 4.0f.xxxx.w == _175.w));
    }
    else
    {
        _178 = false;
    }
    bool _189 = false;
    if (_178)
    {
        float2x2 _181 = returns_float2x2_f22();
        float2 _182 = _181[0];
        float2 _185 = _181[1];
        _189 = all(bool2(float2(2.0f, 0.0f).x == _182.x, float2(2.0f, 0.0f).y == _182.y)) && all(bool2(float2(0.0f, 2.0f).x == _185.x, float2(0.0f, 2.0f).y == _185.y));
    }
    else
    {
        _189 = false;
    }
    bool _204 = false;
    if (_189)
    {
        float3x3 _192 = returns_float3x3_f33();
        float3 _193 = _192[0];
        float3 _196 = _192[1];
        float3 _200 = _192[2];
        _204 = (all(bool3(float3(3.0f, 0.0f, 0.0f).x == _193.x, float3(3.0f, 0.0f, 0.0f).y == _193.y, float3(3.0f, 0.0f, 0.0f).z == _193.z)) && all(bool3(float3(0.0f, 3.0f, 0.0f).x == _196.x, float3(0.0f, 3.0f, 0.0f).y == _196.y, float3(0.0f, 3.0f, 0.0f).z == _196.z))) && all(bool3(float3(0.0f, 0.0f, 3.0f).x == _200.x, float3(0.0f, 0.0f, 3.0f).y == _200.y, float3(0.0f, 0.0f, 3.0f).z == _200.z));
    }
    else
    {
        _204 = false;
    }
    bool _223 = false;
    if (_204)
    {
        float4x4 _207 = returns_float4x4_f44();
        float4 _208 = _207[0];
        float4 _211 = _207[1];
        float4 _215 = _207[2];
        float4 _219 = _207[3];
        _223 = ((all(bool4(float4(4.0f, 0.0f, 0.0f, 0.0f).x == _208.x, float4(4.0f, 0.0f, 0.0f, 0.0f).y == _208.y, float4(4.0f, 0.0f, 0.0f, 0.0f).z == _208.z, float4(4.0f, 0.0f, 0.0f, 0.0f).w == _208.w)) && all(bool4(float4(0.0f, 4.0f, 0.0f, 0.0f).x == _211.x, float4(0.0f, 4.0f, 0.0f, 0.0f).y == _211.y, float4(0.0f, 4.0f, 0.0f, 0.0f).z == _211.z, float4(0.0f, 4.0f, 0.0f, 0.0f).w == _211.w))) && all(bool4(float4(0.0f, 0.0f, 4.0f, 0.0f).x == _215.x, float4(0.0f, 0.0f, 4.0f, 0.0f).y == _215.y, float4(0.0f, 0.0f, 4.0f, 0.0f).z == _215.z, float4(0.0f, 0.0f, 4.0f, 0.0f).w == _215.w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 4.0f).x == _219.x, float4(0.0f, 0.0f, 0.0f, 4.0f).y == _219.y, float4(0.0f, 0.0f, 0.0f, 4.0f).z == _219.z, float4(0.0f, 0.0f, 0.0f, 4.0f).w == _219.w));
    }
    else
    {
        _223 = false;
    }
    bool _228 = false;
    if (_223)
    {
        _228 = 1.0f == returns_half_h();
    }
    else
    {
        _228 = false;
    }
    bool _234 = false;
    if (_228)
    {
        float2 _231 = returns_half2_h2();
        _234 = all(bool2(2.0f.xx.x == _231.x, 2.0f.xx.y == _231.y));
    }
    else
    {
        _234 = false;
    }
    bool _240 = false;
    if (_234)
    {
        float3 _237 = returns_half3_h3();
        _240 = all(bool3(3.0f.xxx.x == _237.x, 3.0f.xxx.y == _237.y, 3.0f.xxx.z == _237.z));
    }
    else
    {
        _240 = false;
    }
    bool _246 = false;
    if (_240)
    {
        float4 _243 = returns_half4_h4();
        _246 = all(bool4(4.0f.xxxx.x == _243.x, 4.0f.xxxx.y == _243.y, 4.0f.xxxx.z == _243.z, 4.0f.xxxx.w == _243.w));
    }
    else
    {
        _246 = false;
    }
    bool _257 = false;
    if (_246)
    {
        float2x2 _249 = returns_half2x2_h22();
        float2 _250 = _249[0];
        float2 _253 = _249[1];
        _257 = all(bool2(float2(2.0f, 0.0f).x == _250.x, float2(2.0f, 0.0f).y == _250.y)) && all(bool2(float2(0.0f, 2.0f).x == _253.x, float2(0.0f, 2.0f).y == _253.y));
    }
    else
    {
        _257 = false;
    }
    bool _272 = false;
    if (_257)
    {
        float3x3 _260 = returns_half3x3_h33();
        float3 _261 = _260[0];
        float3 _264 = _260[1];
        float3 _268 = _260[2];
        _272 = (all(bool3(float3(3.0f, 0.0f, 0.0f).x == _261.x, float3(3.0f, 0.0f, 0.0f).y == _261.y, float3(3.0f, 0.0f, 0.0f).z == _261.z)) && all(bool3(float3(0.0f, 3.0f, 0.0f).x == _264.x, float3(0.0f, 3.0f, 0.0f).y == _264.y, float3(0.0f, 3.0f, 0.0f).z == _264.z))) && all(bool3(float3(0.0f, 0.0f, 3.0f).x == _268.x, float3(0.0f, 0.0f, 3.0f).y == _268.y, float3(0.0f, 0.0f, 3.0f).z == _268.z));
    }
    else
    {
        _272 = false;
    }
    bool _291 = false;
    if (_272)
    {
        float4x4 _275 = returns_half4x4_h44();
        float4 _276 = _275[0];
        float4 _279 = _275[1];
        float4 _283 = _275[2];
        float4 _287 = _275[3];
        _291 = ((all(bool4(float4(4.0f, 0.0f, 0.0f, 0.0f).x == _276.x, float4(4.0f, 0.0f, 0.0f, 0.0f).y == _276.y, float4(4.0f, 0.0f, 0.0f, 0.0f).z == _276.z, float4(4.0f, 0.0f, 0.0f, 0.0f).w == _276.w)) && all(bool4(float4(0.0f, 4.0f, 0.0f, 0.0f).x == _279.x, float4(0.0f, 4.0f, 0.0f, 0.0f).y == _279.y, float4(0.0f, 4.0f, 0.0f, 0.0f).z == _279.z, float4(0.0f, 4.0f, 0.0f, 0.0f).w == _279.w))) && all(bool4(float4(0.0f, 0.0f, 4.0f, 0.0f).x == _283.x, float4(0.0f, 0.0f, 4.0f, 0.0f).y == _283.y, float4(0.0f, 0.0f, 4.0f, 0.0f).z == _283.z, float4(0.0f, 0.0f, 4.0f, 0.0f).w == _283.w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 4.0f).x == _287.x, float4(0.0f, 0.0f, 0.0f, 4.0f).y == _287.y, float4(0.0f, 0.0f, 0.0f, 4.0f).z == _287.z, float4(0.0f, 0.0f, 0.0f, 4.0f).w == _287.w));
    }
    else
    {
        _291 = false;
    }
    bool _296 = false;
    if (_291)
    {
        _296 = true == returns_bool_b();
    }
    else
    {
        _296 = false;
    }
    bool _302 = false;
    if (_296)
    {
        bool2 _299 = returns_bool2_b2();
        _302 = all(bool2(bool2(true, true).x == _299.x, bool2(true, true).y == _299.y));
    }
    else
    {
        _302 = false;
    }
    bool _308 = false;
    if (_302)
    {
        bool3 _305 = returns_bool3_b3();
        _308 = all(bool3(bool3(true, true, true).x == _305.x, bool3(true, true, true).y == _305.y, bool3(true, true, true).z == _305.z));
    }
    else
    {
        _308 = false;
    }
    bool _314 = false;
    if (_308)
    {
        bool4 _311 = returns_bool4_b4();
        _314 = all(bool4(bool4(true, true, true, true).x == _311.x, bool4(true, true, true, true).y == _311.y, bool4(true, true, true, true).z == _311.z, bool4(true, true, true, true).w == _311.w));
    }
    else
    {
        _314 = false;
    }
    bool _319 = false;
    if (_314)
    {
        _319 = 1 == returns_int_i();
    }
    else
    {
        _319 = false;
    }
    bool _325 = false;
    if (_319)
    {
        int2 _322 = returns_int2_i2();
        _325 = all(bool2(int2(2, 2).x == _322.x, int2(2, 2).y == _322.y));
    }
    else
    {
        _325 = false;
    }
    bool _331 = false;
    if (_325)
    {
        int3 _328 = returns_int3_i3();
        _331 = all(bool3(int3(3, 3, 3).x == _328.x, int3(3, 3, 3).y == _328.y, int3(3, 3, 3).z == _328.z));
    }
    else
    {
        _331 = false;
    }
    bool _337 = false;
    if (_331)
    {
        int4 _334 = returns_int4_i4();
        _337 = all(bool4(int4(4, 4, 4, 4).x == _334.x, int4(4, 4, 4, 4).y == _334.y, int4(4, 4, 4, 4).z == _334.z, int4(4, 4, 4, 4).w == _334.w));
    }
    else
    {
        _337 = false;
    }
    float4 _338 = 0.0f.xxxx;
    if (_337)
    {
        _338 = _31_colorGreen;
    }
    else
    {
        _338 = _31_colorRed;
    }
    return _338;
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
