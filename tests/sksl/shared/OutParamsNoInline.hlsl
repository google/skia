cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _32_colorGreen : packoffset(c0);
    float4 _32_colorRed : packoffset(c1);
    float4 _32_colorWhite : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void out_half_vh(out float _47)
{
    _47 = _32_colorWhite.x;
}

void out_half2_vh2(out float2 _56)
{
    _56 = _32_colorWhite.y.xx;
}

void out_half3_vh3(out float3 _65)
{
    _65 = _32_colorWhite.z.xxx;
}

void out_half4_vh4(out float4 _73)
{
    _73 = _32_colorWhite.w.xxxx;
}

void out_half2x2_vh22(out float2x2 _82)
{
    _82 = float2x2(float2(_32_colorWhite.x, 0.0f), float2(0.0f, _32_colorWhite.x));
}

void out_half3x3_vh33(out float3x3 _93)
{
    _93 = float3x3(float3(_32_colorWhite.y, 0.0f, 0.0f), float3(0.0f, _32_colorWhite.y, 0.0f), float3(0.0f, 0.0f, _32_colorWhite.y));
}

void out_half4x4_vh44(out float4x4 _105)
{
    _105 = float4x4(float4(_32_colorWhite.z, 0.0f, 0.0f, 0.0f), float4(0.0f, _32_colorWhite.z, 0.0f, 0.0f), float4(0.0f, 0.0f, _32_colorWhite.z, 0.0f), float4(0.0f, 0.0f, 0.0f, _32_colorWhite.z));
}

void out_int_vi(out int _117)
{
    _117 = int(_32_colorWhite.x);
}

void out_int2_vi2(out int2 _126)
{
    _126 = int(_32_colorWhite.y).xx;
}

void out_int3_vi3(out int3 _136)
{
    _136 = int(_32_colorWhite.z).xxx;
}

void out_int4_vi4(out int4 _146)
{
    _146 = int(_32_colorWhite.w).xxxx;
}

void out_float_vf(out float _153)
{
    _153 = _32_colorWhite.x;
}

void out_float2_vf2(out float2 _158)
{
    _158 = _32_colorWhite.y.xx;
}

void out_float3_vf3(out float3 _164)
{
    _164 = _32_colorWhite.z.xxx;
}

void out_float4_vf4(out float4 _170)
{
    _170 = _32_colorWhite.w.xxxx;
}

void out_float2x2_vf22(out float2x2 _176)
{
    _176 = float2x2(float2(_32_colorWhite.x, 0.0f), float2(0.0f, _32_colorWhite.x));
}

void out_float3x3_vf33(out float3x3 _184)
{
    _184 = float3x3(float3(_32_colorWhite.y, 0.0f, 0.0f), float3(0.0f, _32_colorWhite.y, 0.0f), float3(0.0f, 0.0f, _32_colorWhite.y));
}

void out_float4x4_vf44(out float4x4 _193)
{
    _193 = float4x4(float4(_32_colorWhite.z, 0.0f, 0.0f, 0.0f), float4(0.0f, _32_colorWhite.z, 0.0f, 0.0f), float4(0.0f, 0.0f, _32_colorWhite.z, 0.0f), float4(0.0f, 0.0f, 0.0f, _32_colorWhite.z));
}

void out_bool_vb(out bool _205)
{
    _205 = _32_colorWhite.x != 0.0f;
}

void out_bool2_vb2(out bool2 _214)
{
    _214 = (_32_colorWhite.y != 0.0f).xx;
}

void out_bool3_vb3(out bool3 _224)
{
    _224 = (_32_colorWhite.z != 0.0f).xxx;
}

void out_bool4_vb4(out bool4 _234)
{
    _234 = (_32_colorWhite.w != 0.0f).xxxx;
}

float4 main(float2 _242)
{
    float h = 0.0f;
    out_half_vh(h);
    float2 h2 = 0.0f.xx;
    out_half2_vh2(h2);
    float3 h3 = 0.0f.xxx;
    out_half3_vh3(h3);
    float4 h4 = 0.0f.xxxx;
    out_half4_vh4(h4);
    float _254 = 0.0f;
    out_half_vh(_254);
    h3.y = _254;
    float2 _257 = 0.0f.xx;
    out_half2_vh2(_257);
    h3 = float3(_257.x, h3.y, _257.y);
    float4 _262 = 0.0f.xxxx;
    out_half4_vh4(_262);
    h4 = float4(_262.z, _262.w, _262.x, _262.y);
    float2x2 h2x2 = float2x2(0.0f.xx, 0.0f.xx);
    out_half2x2_vh22(h2x2);
    float3x3 h3x3 = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    out_half3x3_vh33(h3x3);
    float4x4 h4x4 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    out_half4x4_vh44(h4x4);
    float3 _274 = 0.0f.xxx;
    out_half3_vh3(_274);
    h3x3[1] = _274;
    float _280 = 0.0f;
    out_half_vh(_280);
    h4x4[3].w = _280;
    float _286 = 0.0f;
    out_half_vh(_286);
    h2x2[0].x = _286;
    int i = 0;
    out_int_vi(i);
    int2 i2 = int2(0, 0);
    out_int2_vi2(i2);
    int3 i3 = int3(0, 0, 0);
    out_int3_vi3(i3);
    int4 i4 = int4(0, 0, 0, 0);
    out_int4_vi4(i4);
    int3 _297 = int3(0, 0, 0);
    out_int3_vi3(_297);
    i4 = int4(_297.x, _297.y, _297.z, i4.w);
    int _303 = 0;
    out_int_vi(_303);
    i2.y = _303;
    float f = 0.0f;
    out_float_vf(f);
    float2 f2 = 0.0f.xx;
    out_float2_vf2(f2);
    float3 f3 = 0.0f.xxx;
    out_float3_vf3(f3);
    float4 f4 = 0.0f.xxxx;
    out_float4_vf4(f4);
    float2 _314 = 0.0f.xx;
    out_float2_vf2(_314);
    f3 = float3(_314.x, _314.y, f3.z);
    float _320 = 0.0f;
    out_float_vf(_320);
    f2.x = _320;
    float2x2 f2x2 = float2x2(0.0f.xx, 0.0f.xx);
    out_float2x2_vf22(f2x2);
    float3x3 f3x3 = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    out_float3x3_vf33(f3x3);
    float4x4 f4x4 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    out_float4x4_vf44(f4x4);
    float _331 = 0.0f;
    out_float_vf(_331);
    f2x2[0].x = _331;
    bool b = false;
    out_bool_vb(b);
    bool2 b2 = bool2(false, false);
    out_bool2_vb2(b2);
    bool3 b3 = bool3(false, false, false);
    out_bool3_vb3(b3);
    bool4 b4 = bool4(false, false, false, false);
    out_bool4_vb4(b4);
    bool2 _342 = bool2(false, false);
    out_bool2_vb2(_342);
    b4 = bool4(_342.x, b4.y, b4.z, _342.y);
    bool _348 = false;
    out_bool_vb(_348);
    b3.z = _348;
    bool ok = true;
    bool _381 = false;
    if (ok)
    {
        _381 = 1.0f == ((((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x);
    }
    else
    {
        _381 = false;
    }
    ok = _381;
    bool _408 = false;
    if (ok)
    {
        _408 = 1.0f == ((((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x);
    }
    else
    {
        _408 = false;
    }
    ok = _408;
    bool _423 = false;
    if (ok)
    {
        _423 = 1 == (((i * i2.x) * i3.x) * i4.x);
    }
    else
    {
        _423 = false;
    }
    ok = _423;
    bool _443 = false;
    if (ok)
    {
        bool _432 = false;
        if (b)
        {
            _432 = b2.x;
        }
        else
        {
            _432 = false;
        }
        bool _437 = false;
        if (_432)
        {
            _437 = b3.x;
        }
        else
        {
            _437 = false;
        }
        bool _442 = false;
        if (_437)
        {
            _442 = b4.x;
        }
        else
        {
            _442 = false;
        }
        _443 = _442;
    }
    else
    {
        _443 = false;
    }
    ok = _443;
    float4 _445 = 0.0f.xxxx;
    if (ok)
    {
        _445 = _32_colorGreen;
    }
    else
    {
        _445 = _32_colorRed;
    }
    return _445;
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
