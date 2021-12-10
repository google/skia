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
    float _245 = 0.0f;
    out_half_vh(_245);
    float h = _245;
    float2 _249 = 0.0f.xx;
    out_half2_vh2(_249);
    float2 h2 = _249;
    float3 _253 = 0.0f.xxx;
    out_half3_vh3(_253);
    float3 h3 = _253;
    float4 _257 = 0.0f.xxxx;
    out_half4_vh4(_257);
    float4 h4 = _257;
    float _262 = 0.0f;
    out_half_vh(_262);
    h3.y = _262;
    float2 _265 = 0.0f.xx;
    out_half2_vh2(_265);
    h3 = float3(_265.x, h3.y, _265.y);
    float4 _270 = 0.0f.xxxx;
    out_half4_vh4(_270);
    h4 = float4(_270.z, _270.w, _270.x, _270.y);
    float2x2 _276 = float2x2(0.0f.xx, 0.0f.xx);
    out_half2x2_vh22(_276);
    float2x2 h2x2 = _276;
    float3x3 _280 = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    out_half3x3_vh33(_280);
    float3x3 h3x3 = _280;
    float4x4 _284 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    out_half4x4_vh44(_284);
    float4x4 h4x4 = _284;
    float3 _288 = 0.0f.xxx;
    out_half3_vh3(_288);
    h3x3[1] = _288;
    float _294 = 0.0f;
    out_half_vh(_294);
    h4x4[3].w = _294;
    float _300 = 0.0f;
    out_half_vh(_300);
    h2x2[0].x = _300;
    int _304 = 0;
    out_int_vi(_304);
    int i = _304;
    int2 _308 = int2(0, 0);
    out_int2_vi2(_308);
    int2 i2 = _308;
    int3 _312 = int3(0, 0, 0);
    out_int3_vi3(_312);
    int3 i3 = _312;
    int4 _316 = int4(0, 0, 0, 0);
    out_int4_vi4(_316);
    int4 i4 = _316;
    int3 _319 = int3(0, 0, 0);
    out_int3_vi3(_319);
    i4 = int4(_319.x, _319.y, _319.z, i4.w);
    int _325 = 0;
    out_int_vi(_325);
    i2.y = _325;
    float _329 = 0.0f;
    out_float_vf(_329);
    float f = _329;
    float2 _333 = 0.0f.xx;
    out_float2_vf2(_333);
    float2 f2 = _333;
    float3 _337 = 0.0f.xxx;
    out_float3_vf3(_337);
    float3 f3 = _337;
    float4 _341 = 0.0f.xxxx;
    out_float4_vf4(_341);
    float4 f4 = _341;
    float2 _344 = 0.0f.xx;
    out_float2_vf2(_344);
    f3 = float3(_344.x, _344.y, f3.z);
    float _350 = 0.0f;
    out_float_vf(_350);
    f2.x = _350;
    float2x2 _354 = float2x2(0.0f.xx, 0.0f.xx);
    out_float2x2_vf22(_354);
    float2x2 f2x2 = _354;
    float3x3 _358 = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    out_float3x3_vf33(_358);
    float3x3 f3x3 = _358;
    float4x4 _362 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    out_float4x4_vf44(_362);
    float4x4 f4x4 = _362;
    float _367 = 0.0f;
    out_float_vf(_367);
    f2x2[0].x = _367;
    bool _371 = false;
    out_bool_vb(_371);
    bool b = _371;
    bool2 _375 = bool2(false, false);
    out_bool2_vb2(_375);
    bool2 b2 = _375;
    bool3 _379 = bool3(false, false, false);
    out_bool3_vb3(_379);
    bool3 b3 = _379;
    bool4 _383 = bool4(false, false, false, false);
    out_bool4_vb4(_383);
    bool4 b4 = _383;
    bool2 _386 = bool2(false, false);
    out_bool2_vb2(_386);
    b4 = bool4(_386.x, b4.y, b4.z, _386.y);
    bool _392 = false;
    out_bool_vb(_392);
    b3.z = _392;
    bool ok = true;
    bool _424 = false;
    if (true)
    {
        _424 = 1.0f == ((((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x);
    }
    else
    {
        _424 = false;
    }
    ok = _424;
    bool _450 = false;
    if (_424)
    {
        _450 = 1.0f == ((((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x);
    }
    else
    {
        _450 = false;
    }
    ok = _450;
    bool _464 = false;
    if (_450)
    {
        _464 = 1 == (((i * i2.x) * i3.x) * i4.x);
    }
    else
    {
        _464 = false;
    }
    ok = _464;
    bool _483 = false;
    if (_464)
    {
        bool _472 = false;
        if (b)
        {
            _472 = b2.x;
        }
        else
        {
            _472 = false;
        }
        bool _477 = false;
        if (_472)
        {
            _477 = b3.x;
        }
        else
        {
            _477 = false;
        }
        bool _482 = false;
        if (_477)
        {
            _482 = b4.x;
        }
        else
        {
            _482 = false;
        }
        _483 = _482;
    }
    else
    {
        _483 = false;
    }
    ok = _483;
    float4 _484 = 0.0f.xxxx;
    if (_483)
    {
        _484 = _32_colorGreen;
    }
    else
    {
        _484 = _32_colorRed;
    }
    return _484;
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
