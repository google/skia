cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _29_colorGreen : packoffset(c0);
    float4 _29_colorRed : packoffset(c1);
    float4 _29_colorWhite : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void out_half_vh(out float _44)
{
    _44 = _29_colorWhite.x;
}

void out_half2_vh2(out float2 _53)
{
    _53 = _29_colorWhite.y.xx;
}

void out_half3_vh3(out float3 _62)
{
    _62 = _29_colorWhite.z.xxx;
}

void out_half4_vh4(out float4 _70)
{
    _70 = _29_colorWhite.w.xxxx;
}

void out_half2x2_vh22(out float2x2 _79)
{
    _79 = float2x2(float2(_29_colorWhite.x, 0.0f), float2(0.0f, _29_colorWhite.x));
}

void out_half3x3_vh33(out float3x3 _90)
{
    _90 = float3x3(float3(_29_colorWhite.y, 0.0f, 0.0f), float3(0.0f, _29_colorWhite.y, 0.0f), float3(0.0f, 0.0f, _29_colorWhite.y));
}

void out_half4x4_vh44(out float4x4 _102)
{
    _102 = float4x4(float4(_29_colorWhite.z, 0.0f, 0.0f, 0.0f), float4(0.0f, _29_colorWhite.z, 0.0f, 0.0f), float4(0.0f, 0.0f, _29_colorWhite.z, 0.0f), float4(0.0f, 0.0f, 0.0f, _29_colorWhite.z));
}

void out_int_vi(out int _114)
{
    _114 = int(_29_colorWhite.x);
}

void out_int2_vi2(out int2 _123)
{
    _123 = int(_29_colorWhite.y).xx;
}

void out_int3_vi3(out int3 _133)
{
    _133 = int(_29_colorWhite.z).xxx;
}

void out_int4_vi4(out int4 _143)
{
    _143 = int(_29_colorWhite.w).xxxx;
}

void out_float_vf(out float _150)
{
    _150 = _29_colorWhite.x;
}

void out_float2_vf2(out float2 _155)
{
    _155 = _29_colorWhite.y.xx;
}

void out_float3_vf3(out float3 _161)
{
    _161 = _29_colorWhite.z.xxx;
}

void out_float4_vf4(out float4 _167)
{
    _167 = _29_colorWhite.w.xxxx;
}

void out_float2x2_vf22(out float2x2 _173)
{
    _173 = float2x2(float2(_29_colorWhite.x, 0.0f), float2(0.0f, _29_colorWhite.x));
}

void out_float3x3_vf33(out float3x3 _181)
{
    _181 = float3x3(float3(_29_colorWhite.y, 0.0f, 0.0f), float3(0.0f, _29_colorWhite.y, 0.0f), float3(0.0f, 0.0f, _29_colorWhite.y));
}

void out_float4x4_vf44(out float4x4 _190)
{
    _190 = float4x4(float4(_29_colorWhite.z, 0.0f, 0.0f, 0.0f), float4(0.0f, _29_colorWhite.z, 0.0f, 0.0f), float4(0.0f, 0.0f, _29_colorWhite.z, 0.0f), float4(0.0f, 0.0f, 0.0f, _29_colorWhite.z));
}

void out_bool_vb(out bool _203)
{
    _203 = _29_colorWhite.x != 0.0f;
}

void out_bool2_vb2(out bool2 _212)
{
    _212 = (_29_colorWhite.y != 0.0f).xx;
}

void out_bool3_vb3(out bool3 _222)
{
    _222 = (_29_colorWhite.z != 0.0f).xxx;
}

void out_bool4_vb4(out bool4 _232)
{
    _232 = (_29_colorWhite.w != 0.0f).xxxx;
}

float4 main(float2 _240)
{
    float _243 = 0.0f;
    out_half_vh(_243);
    float h = _243;
    float2 _247 = 0.0f.xx;
    out_half2_vh2(_247);
    float2 h2 = _247;
    float3 _251 = 0.0f.xxx;
    out_half3_vh3(_251);
    float3 h3 = _251;
    float4 _255 = 0.0f.xxxx;
    out_half4_vh4(_255);
    float4 h4 = _255;
    float _260 = 0.0f;
    out_half_vh(_260);
    h3.y = _260;
    float2 _263 = 0.0f.xx;
    out_half2_vh2(_263);
    h3 = float3(_263.x, h3.y, _263.y);
    float4 _268 = 0.0f.xxxx;
    out_half4_vh4(_268);
    h4 = float4(_268.z, _268.w, _268.x, _268.y);
    float2x2 _274 = float2x2(0.0f.xx, 0.0f.xx);
    out_half2x2_vh22(_274);
    float2x2 h2x2 = _274;
    float3x3 _278 = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    out_half3x3_vh33(_278);
    float3x3 h3x3 = _278;
    float4x4 _282 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    out_half4x4_vh44(_282);
    float4x4 h4x4 = _282;
    float3 _286 = 0.0f.xxx;
    out_half3_vh3(_286);
    h3x3[1] = _286;
    float _292 = 0.0f;
    out_half_vh(_292);
    h4x4[3].w = _292;
    float _298 = 0.0f;
    out_half_vh(_298);
    h2x2[0].x = _298;
    int _302 = 0;
    out_int_vi(_302);
    int i = _302;
    int2 _306 = int2(0, 0);
    out_int2_vi2(_306);
    int2 i2 = _306;
    int3 _310 = int3(0, 0, 0);
    out_int3_vi3(_310);
    int3 i3 = _310;
    int4 _314 = int4(0, 0, 0, 0);
    out_int4_vi4(_314);
    int4 i4 = _314;
    int3 _317 = int3(0, 0, 0);
    out_int3_vi3(_317);
    i4 = int4(_317.x, _317.y, _317.z, i4.w);
    int _323 = 0;
    out_int_vi(_323);
    i2.y = _323;
    float _327 = 0.0f;
    out_float_vf(_327);
    float f = _327;
    float2 _331 = 0.0f.xx;
    out_float2_vf2(_331);
    float2 f2 = _331;
    float3 _335 = 0.0f.xxx;
    out_float3_vf3(_335);
    float3 f3 = _335;
    float4 _339 = 0.0f.xxxx;
    out_float4_vf4(_339);
    float4 f4 = _339;
    float2 _342 = 0.0f.xx;
    out_float2_vf2(_342);
    f3 = float3(_342.x, _342.y, f3.z);
    float _348 = 0.0f;
    out_float_vf(_348);
    f2.x = _348;
    float2x2 _352 = float2x2(0.0f.xx, 0.0f.xx);
    out_float2x2_vf22(_352);
    float2x2 f2x2 = _352;
    float3x3 _356 = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    out_float3x3_vf33(_356);
    float3x3 f3x3 = _356;
    float4x4 _360 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    out_float4x4_vf44(_360);
    float4x4 f4x4 = _360;
    float _365 = 0.0f;
    out_float_vf(_365);
    f2x2[0].x = _365;
    bool _369 = false;
    out_bool_vb(_369);
    bool b = _369;
    bool2 _373 = bool2(false, false);
    out_bool2_vb2(_373);
    bool2 b2 = _373;
    bool3 _377 = bool3(false, false, false);
    out_bool3_vb3(_377);
    bool3 b3 = _377;
    bool4 _381 = bool4(false, false, false, false);
    out_bool4_vb4(_381);
    bool4 b4 = _381;
    bool2 _384 = bool2(false, false);
    out_bool2_vb2(_384);
    b4 = bool4(_384.x, b4.y, b4.z, _384.y);
    bool _390 = false;
    out_bool_vb(_390);
    b3.z = _390;
    bool ok = true;
    bool _422 = false;
    if (true)
    {
        _422 = 1.0f == ((((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x);
    }
    else
    {
        _422 = false;
    }
    ok = _422;
    bool _448 = false;
    if (_422)
    {
        _448 = 1.0f == ((((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x);
    }
    else
    {
        _448 = false;
    }
    ok = _448;
    bool _462 = false;
    if (_448)
    {
        _462 = 1 == (((i * i2.x) * i3.x) * i4.x);
    }
    else
    {
        _462 = false;
    }
    ok = _462;
    bool _481 = false;
    if (_462)
    {
        bool _470 = false;
        if (b)
        {
            _470 = b2.x;
        }
        else
        {
            _470 = false;
        }
        bool _475 = false;
        if (_470)
        {
            _475 = b3.x;
        }
        else
        {
            _475 = false;
        }
        bool _480 = false;
        if (_475)
        {
            _480 = b4.x;
        }
        else
        {
            _480 = false;
        }
        _481 = _480;
    }
    else
    {
        _481 = false;
    }
    ok = _481;
    float4 _482 = 0.0f.xxxx;
    if (_481)
    {
        _482 = _29_colorGreen;
    }
    else
    {
        _482 = _29_colorRed;
    }
    return _482;
}

void frag_main()
{
    float2 _39 = 0.0f.xx;
    sk_FragColor = main(_39);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
