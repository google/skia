cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _28_colorGreen : packoffset(c0);
    float4 _28_colorRed : packoffset(c1);
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

float4 main(float2 _120)
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
    bool _164 = false;
    if (true)
    {
        float2 _161 = returns_float2_f2();
        _164 = all(bool2(2.0f.xx.x == _161.x, 2.0f.xx.y == _161.y));
    }
    else
    {
        _164 = false;
    }
    bool _170 = false;
    if (_164)
    {
        float3 _167 = returns_float3_f3();
        _170 = all(bool3(3.0f.xxx.x == _167.x, 3.0f.xxx.y == _167.y, 3.0f.xxx.z == _167.z));
    }
    else
    {
        _170 = false;
    }
    bool _176 = false;
    if (_170)
    {
        float4 _173 = returns_float4_f4();
        _176 = all(bool4(4.0f.xxxx.x == _173.x, 4.0f.xxxx.y == _173.y, 4.0f.xxxx.z == _173.z, 4.0f.xxxx.w == _173.w));
    }
    else
    {
        _176 = false;
    }
    bool _187 = false;
    if (_176)
    {
        float2x2 _179 = returns_float2x2_f22();
        float2 _180 = _179[0];
        float2 _183 = _179[1];
        _187 = all(bool2(float2(2.0f, 0.0f).x == _180.x, float2(2.0f, 0.0f).y == _180.y)) && all(bool2(float2(0.0f, 2.0f).x == _183.x, float2(0.0f, 2.0f).y == _183.y));
    }
    else
    {
        _187 = false;
    }
    bool _202 = false;
    if (_187)
    {
        float3x3 _190 = returns_float3x3_f33();
        float3 _191 = _190[0];
        float3 _194 = _190[1];
        float3 _198 = _190[2];
        _202 = (all(bool3(float3(3.0f, 0.0f, 0.0f).x == _191.x, float3(3.0f, 0.0f, 0.0f).y == _191.y, float3(3.0f, 0.0f, 0.0f).z == _191.z)) && all(bool3(float3(0.0f, 3.0f, 0.0f).x == _194.x, float3(0.0f, 3.0f, 0.0f).y == _194.y, float3(0.0f, 3.0f, 0.0f).z == _194.z))) && all(bool3(float3(0.0f, 0.0f, 3.0f).x == _198.x, float3(0.0f, 0.0f, 3.0f).y == _198.y, float3(0.0f, 0.0f, 3.0f).z == _198.z));
    }
    else
    {
        _202 = false;
    }
    bool _221 = false;
    if (_202)
    {
        float4x4 _205 = returns_float4x4_f44();
        float4 _206 = _205[0];
        float4 _209 = _205[1];
        float4 _213 = _205[2];
        float4 _217 = _205[3];
        _221 = ((all(bool4(float4(4.0f, 0.0f, 0.0f, 0.0f).x == _206.x, float4(4.0f, 0.0f, 0.0f, 0.0f).y == _206.y, float4(4.0f, 0.0f, 0.0f, 0.0f).z == _206.z, float4(4.0f, 0.0f, 0.0f, 0.0f).w == _206.w)) && all(bool4(float4(0.0f, 4.0f, 0.0f, 0.0f).x == _209.x, float4(0.0f, 4.0f, 0.0f, 0.0f).y == _209.y, float4(0.0f, 4.0f, 0.0f, 0.0f).z == _209.z, float4(0.0f, 4.0f, 0.0f, 0.0f).w == _209.w))) && all(bool4(float4(0.0f, 0.0f, 4.0f, 0.0f).x == _213.x, float4(0.0f, 0.0f, 4.0f, 0.0f).y == _213.y, float4(0.0f, 0.0f, 4.0f, 0.0f).z == _213.z, float4(0.0f, 0.0f, 4.0f, 0.0f).w == _213.w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 4.0f).x == _217.x, float4(0.0f, 0.0f, 0.0f, 4.0f).y == _217.y, float4(0.0f, 0.0f, 0.0f, 4.0f).z == _217.z, float4(0.0f, 0.0f, 0.0f, 4.0f).w == _217.w));
    }
    else
    {
        _221 = false;
    }
    bool _226 = false;
    if (_221)
    {
        _226 = 1.0f == returns_half_h();
    }
    else
    {
        _226 = false;
    }
    bool _232 = false;
    if (_226)
    {
        float2 _229 = returns_half2_h2();
        _232 = all(bool2(2.0f.xx.x == _229.x, 2.0f.xx.y == _229.y));
    }
    else
    {
        _232 = false;
    }
    bool _238 = false;
    if (_232)
    {
        float3 _235 = returns_half3_h3();
        _238 = all(bool3(3.0f.xxx.x == _235.x, 3.0f.xxx.y == _235.y, 3.0f.xxx.z == _235.z));
    }
    else
    {
        _238 = false;
    }
    bool _244 = false;
    if (_238)
    {
        float4 _241 = returns_half4_h4();
        _244 = all(bool4(4.0f.xxxx.x == _241.x, 4.0f.xxxx.y == _241.y, 4.0f.xxxx.z == _241.z, 4.0f.xxxx.w == _241.w));
    }
    else
    {
        _244 = false;
    }
    bool _255 = false;
    if (_244)
    {
        float2x2 _247 = returns_half2x2_h22();
        float2 _248 = _247[0];
        float2 _251 = _247[1];
        _255 = all(bool2(float2(2.0f, 0.0f).x == _248.x, float2(2.0f, 0.0f).y == _248.y)) && all(bool2(float2(0.0f, 2.0f).x == _251.x, float2(0.0f, 2.0f).y == _251.y));
    }
    else
    {
        _255 = false;
    }
    bool _270 = false;
    if (_255)
    {
        float3x3 _258 = returns_half3x3_h33();
        float3 _259 = _258[0];
        float3 _262 = _258[1];
        float3 _266 = _258[2];
        _270 = (all(bool3(float3(3.0f, 0.0f, 0.0f).x == _259.x, float3(3.0f, 0.0f, 0.0f).y == _259.y, float3(3.0f, 0.0f, 0.0f).z == _259.z)) && all(bool3(float3(0.0f, 3.0f, 0.0f).x == _262.x, float3(0.0f, 3.0f, 0.0f).y == _262.y, float3(0.0f, 3.0f, 0.0f).z == _262.z))) && all(bool3(float3(0.0f, 0.0f, 3.0f).x == _266.x, float3(0.0f, 0.0f, 3.0f).y == _266.y, float3(0.0f, 0.0f, 3.0f).z == _266.z));
    }
    else
    {
        _270 = false;
    }
    bool _289 = false;
    if (_270)
    {
        float4x4 _273 = returns_half4x4_h44();
        float4 _274 = _273[0];
        float4 _277 = _273[1];
        float4 _281 = _273[2];
        float4 _285 = _273[3];
        _289 = ((all(bool4(float4(4.0f, 0.0f, 0.0f, 0.0f).x == _274.x, float4(4.0f, 0.0f, 0.0f, 0.0f).y == _274.y, float4(4.0f, 0.0f, 0.0f, 0.0f).z == _274.z, float4(4.0f, 0.0f, 0.0f, 0.0f).w == _274.w)) && all(bool4(float4(0.0f, 4.0f, 0.0f, 0.0f).x == _277.x, float4(0.0f, 4.0f, 0.0f, 0.0f).y == _277.y, float4(0.0f, 4.0f, 0.0f, 0.0f).z == _277.z, float4(0.0f, 4.0f, 0.0f, 0.0f).w == _277.w))) && all(bool4(float4(0.0f, 0.0f, 4.0f, 0.0f).x == _281.x, float4(0.0f, 0.0f, 4.0f, 0.0f).y == _281.y, float4(0.0f, 0.0f, 4.0f, 0.0f).z == _281.z, float4(0.0f, 0.0f, 4.0f, 0.0f).w == _281.w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 4.0f).x == _285.x, float4(0.0f, 0.0f, 0.0f, 4.0f).y == _285.y, float4(0.0f, 0.0f, 0.0f, 4.0f).z == _285.z, float4(0.0f, 0.0f, 0.0f, 4.0f).w == _285.w));
    }
    else
    {
        _289 = false;
    }
    bool _294 = false;
    if (_289)
    {
        _294 = true == returns_bool_b();
    }
    else
    {
        _294 = false;
    }
    bool _300 = false;
    if (_294)
    {
        bool2 _297 = returns_bool2_b2();
        _300 = all(bool2(bool2(true, true).x == _297.x, bool2(true, true).y == _297.y));
    }
    else
    {
        _300 = false;
    }
    bool _306 = false;
    if (_300)
    {
        bool3 _303 = returns_bool3_b3();
        _306 = all(bool3(bool3(true, true, true).x == _303.x, bool3(true, true, true).y == _303.y, bool3(true, true, true).z == _303.z));
    }
    else
    {
        _306 = false;
    }
    bool _312 = false;
    if (_306)
    {
        bool4 _309 = returns_bool4_b4();
        _312 = all(bool4(bool4(true, true, true, true).x == _309.x, bool4(true, true, true, true).y == _309.y, bool4(true, true, true, true).z == _309.z, bool4(true, true, true, true).w == _309.w));
    }
    else
    {
        _312 = false;
    }
    bool _317 = false;
    if (_312)
    {
        _317 = 1 == returns_int_i();
    }
    else
    {
        _317 = false;
    }
    bool _323 = false;
    if (_317)
    {
        int2 _320 = returns_int2_i2();
        _323 = all(bool2(int2(2, 2).x == _320.x, int2(2, 2).y == _320.y));
    }
    else
    {
        _323 = false;
    }
    bool _329 = false;
    if (_323)
    {
        int3 _326 = returns_int3_i3();
        _329 = all(bool3(int3(3, 3, 3).x == _326.x, int3(3, 3, 3).y == _326.y, int3(3, 3, 3).z == _326.z));
    }
    else
    {
        _329 = false;
    }
    bool _335 = false;
    if (_329)
    {
        int4 _332 = returns_int4_i4();
        _335 = all(bool4(int4(4, 4, 4, 4).x == _332.x, int4(4, 4, 4, 4).y == _332.y, int4(4, 4, 4, 4).z == _332.z, int4(4, 4, 4, 4).w == _332.w));
    }
    else
    {
        _335 = false;
    }
    float4 _336 = 0.0f.xxxx;
    if (_335)
    {
        _336 = _28_colorGreen;
    }
    else
    {
        _336 = _28_colorRed;
    }
    return _336;
}

void frag_main()
{
    float2 _38 = 0.0f.xx;
    sk_FragColor = main(_38);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
