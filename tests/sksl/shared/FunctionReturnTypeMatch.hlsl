cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _32_colorGreen : packoffset(c0);
    float4 _32_colorRed : packoffset(c1);
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

float4 main(float2 _123)
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
    bool _167 = false;
    if (true)
    {
        float2 _164 = returns_float2_f2();
        _167 = all(bool2(2.0f.xx.x == _164.x, 2.0f.xx.y == _164.y));
    }
    else
    {
        _167 = false;
    }
    bool _173 = false;
    if (_167)
    {
        float3 _170 = returns_float3_f3();
        _173 = all(bool3(3.0f.xxx.x == _170.x, 3.0f.xxx.y == _170.y, 3.0f.xxx.z == _170.z));
    }
    else
    {
        _173 = false;
    }
    bool _179 = false;
    if (_173)
    {
        float4 _176 = returns_float4_f4();
        _179 = all(bool4(4.0f.xxxx.x == _176.x, 4.0f.xxxx.y == _176.y, 4.0f.xxxx.z == _176.z, 4.0f.xxxx.w == _176.w));
    }
    else
    {
        _179 = false;
    }
    bool _190 = false;
    if (_179)
    {
        float2x2 _182 = returns_float2x2_f22();
        float2 _183 = _182[0];
        float2 _186 = _182[1];
        _190 = all(bool2(float2(2.0f, 0.0f).x == _183.x, float2(2.0f, 0.0f).y == _183.y)) && all(bool2(float2(0.0f, 2.0f).x == _186.x, float2(0.0f, 2.0f).y == _186.y));
    }
    else
    {
        _190 = false;
    }
    bool _205 = false;
    if (_190)
    {
        float3x3 _193 = returns_float3x3_f33();
        float3 _194 = _193[0];
        float3 _197 = _193[1];
        float3 _201 = _193[2];
        _205 = (all(bool3(float3(3.0f, 0.0f, 0.0f).x == _194.x, float3(3.0f, 0.0f, 0.0f).y == _194.y, float3(3.0f, 0.0f, 0.0f).z == _194.z)) && all(bool3(float3(0.0f, 3.0f, 0.0f).x == _197.x, float3(0.0f, 3.0f, 0.0f).y == _197.y, float3(0.0f, 3.0f, 0.0f).z == _197.z))) && all(bool3(float3(0.0f, 0.0f, 3.0f).x == _201.x, float3(0.0f, 0.0f, 3.0f).y == _201.y, float3(0.0f, 0.0f, 3.0f).z == _201.z));
    }
    else
    {
        _205 = false;
    }
    bool _224 = false;
    if (_205)
    {
        float4x4 _208 = returns_float4x4_f44();
        float4 _209 = _208[0];
        float4 _212 = _208[1];
        float4 _216 = _208[2];
        float4 _220 = _208[3];
        _224 = ((all(bool4(float4(4.0f, 0.0f, 0.0f, 0.0f).x == _209.x, float4(4.0f, 0.0f, 0.0f, 0.0f).y == _209.y, float4(4.0f, 0.0f, 0.0f, 0.0f).z == _209.z, float4(4.0f, 0.0f, 0.0f, 0.0f).w == _209.w)) && all(bool4(float4(0.0f, 4.0f, 0.0f, 0.0f).x == _212.x, float4(0.0f, 4.0f, 0.0f, 0.0f).y == _212.y, float4(0.0f, 4.0f, 0.0f, 0.0f).z == _212.z, float4(0.0f, 4.0f, 0.0f, 0.0f).w == _212.w))) && all(bool4(float4(0.0f, 0.0f, 4.0f, 0.0f).x == _216.x, float4(0.0f, 0.0f, 4.0f, 0.0f).y == _216.y, float4(0.0f, 0.0f, 4.0f, 0.0f).z == _216.z, float4(0.0f, 0.0f, 4.0f, 0.0f).w == _216.w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 4.0f).x == _220.x, float4(0.0f, 0.0f, 0.0f, 4.0f).y == _220.y, float4(0.0f, 0.0f, 0.0f, 4.0f).z == _220.z, float4(0.0f, 0.0f, 0.0f, 4.0f).w == _220.w));
    }
    else
    {
        _224 = false;
    }
    bool _229 = false;
    if (_224)
    {
        _229 = 1.0f == returns_half_h();
    }
    else
    {
        _229 = false;
    }
    bool _235 = false;
    if (_229)
    {
        float2 _232 = returns_half2_h2();
        _235 = all(bool2(2.0f.xx.x == _232.x, 2.0f.xx.y == _232.y));
    }
    else
    {
        _235 = false;
    }
    bool _241 = false;
    if (_235)
    {
        float3 _238 = returns_half3_h3();
        _241 = all(bool3(3.0f.xxx.x == _238.x, 3.0f.xxx.y == _238.y, 3.0f.xxx.z == _238.z));
    }
    else
    {
        _241 = false;
    }
    bool _247 = false;
    if (_241)
    {
        float4 _244 = returns_half4_h4();
        _247 = all(bool4(4.0f.xxxx.x == _244.x, 4.0f.xxxx.y == _244.y, 4.0f.xxxx.z == _244.z, 4.0f.xxxx.w == _244.w));
    }
    else
    {
        _247 = false;
    }
    bool _258 = false;
    if (_247)
    {
        float2x2 _250 = returns_half2x2_h22();
        float2 _251 = _250[0];
        float2 _254 = _250[1];
        _258 = all(bool2(float2(2.0f, 0.0f).x == _251.x, float2(2.0f, 0.0f).y == _251.y)) && all(bool2(float2(0.0f, 2.0f).x == _254.x, float2(0.0f, 2.0f).y == _254.y));
    }
    else
    {
        _258 = false;
    }
    bool _273 = false;
    if (_258)
    {
        float3x3 _261 = returns_half3x3_h33();
        float3 _262 = _261[0];
        float3 _265 = _261[1];
        float3 _269 = _261[2];
        _273 = (all(bool3(float3(3.0f, 0.0f, 0.0f).x == _262.x, float3(3.0f, 0.0f, 0.0f).y == _262.y, float3(3.0f, 0.0f, 0.0f).z == _262.z)) && all(bool3(float3(0.0f, 3.0f, 0.0f).x == _265.x, float3(0.0f, 3.0f, 0.0f).y == _265.y, float3(0.0f, 3.0f, 0.0f).z == _265.z))) && all(bool3(float3(0.0f, 0.0f, 3.0f).x == _269.x, float3(0.0f, 0.0f, 3.0f).y == _269.y, float3(0.0f, 0.0f, 3.0f).z == _269.z));
    }
    else
    {
        _273 = false;
    }
    bool _292 = false;
    if (_273)
    {
        float4x4 _276 = returns_half4x4_h44();
        float4 _277 = _276[0];
        float4 _280 = _276[1];
        float4 _284 = _276[2];
        float4 _288 = _276[3];
        _292 = ((all(bool4(float4(4.0f, 0.0f, 0.0f, 0.0f).x == _277.x, float4(4.0f, 0.0f, 0.0f, 0.0f).y == _277.y, float4(4.0f, 0.0f, 0.0f, 0.0f).z == _277.z, float4(4.0f, 0.0f, 0.0f, 0.0f).w == _277.w)) && all(bool4(float4(0.0f, 4.0f, 0.0f, 0.0f).x == _280.x, float4(0.0f, 4.0f, 0.0f, 0.0f).y == _280.y, float4(0.0f, 4.0f, 0.0f, 0.0f).z == _280.z, float4(0.0f, 4.0f, 0.0f, 0.0f).w == _280.w))) && all(bool4(float4(0.0f, 0.0f, 4.0f, 0.0f).x == _284.x, float4(0.0f, 0.0f, 4.0f, 0.0f).y == _284.y, float4(0.0f, 0.0f, 4.0f, 0.0f).z == _284.z, float4(0.0f, 0.0f, 4.0f, 0.0f).w == _284.w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 4.0f).x == _288.x, float4(0.0f, 0.0f, 0.0f, 4.0f).y == _288.y, float4(0.0f, 0.0f, 0.0f, 4.0f).z == _288.z, float4(0.0f, 0.0f, 0.0f, 4.0f).w == _288.w));
    }
    else
    {
        _292 = false;
    }
    bool _297 = false;
    if (_292)
    {
        _297 = true == returns_bool_b();
    }
    else
    {
        _297 = false;
    }
    bool _303 = false;
    if (_297)
    {
        bool2 _300 = returns_bool2_b2();
        _303 = all(bool2(bool2(true, true).x == _300.x, bool2(true, true).y == _300.y));
    }
    else
    {
        _303 = false;
    }
    bool _309 = false;
    if (_303)
    {
        bool3 _306 = returns_bool3_b3();
        _309 = all(bool3(bool3(true, true, true).x == _306.x, bool3(true, true, true).y == _306.y, bool3(true, true, true).z == _306.z));
    }
    else
    {
        _309 = false;
    }
    bool _315 = false;
    if (_309)
    {
        bool4 _312 = returns_bool4_b4();
        _315 = all(bool4(bool4(true, true, true, true).x == _312.x, bool4(true, true, true, true).y == _312.y, bool4(true, true, true, true).z == _312.z, bool4(true, true, true, true).w == _312.w));
    }
    else
    {
        _315 = false;
    }
    bool _320 = false;
    if (_315)
    {
        _320 = 1 == returns_int_i();
    }
    else
    {
        _320 = false;
    }
    bool _326 = false;
    if (_320)
    {
        int2 _323 = returns_int2_i2();
        _326 = all(bool2(int2(2, 2).x == _323.x, int2(2, 2).y == _323.y));
    }
    else
    {
        _326 = false;
    }
    bool _332 = false;
    if (_326)
    {
        int3 _329 = returns_int3_i3();
        _332 = all(bool3(int3(3, 3, 3).x == _329.x, int3(3, 3, 3).y == _329.y, int3(3, 3, 3).z == _329.z));
    }
    else
    {
        _332 = false;
    }
    bool _338 = false;
    if (_332)
    {
        int4 _335 = returns_int4_i4();
        _338 = all(bool4(int4(4, 4, 4, 4).x == _335.x, int4(4, 4, 4, 4).y == _335.y, int4(4, 4, 4, 4).z == _335.z, int4(4, 4, 4, 4).w == _335.w));
    }
    else
    {
        _338 = false;
    }
    float4 _339 = 0.0f.xxxx;
    if (_338)
    {
        _339 = _32_colorGreen;
    }
    else
    {
        _339 = _32_colorRed;
    }
    return _339;
}

void frag_main()
{
    float2 _42 = 0.0f.xx;
    sk_FragColor = main(_42);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
