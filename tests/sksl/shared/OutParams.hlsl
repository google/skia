cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _33_colorGreen : packoffset(c0);
    float4 _33_colorRed : packoffset(c1);
    float4 _33_colorWhite : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void out_half_vh(out float _48)
{
    _48 = _33_colorWhite.x;
}

void out_half2_vh2(out float2 _56)
{
    _56 = _33_colorWhite.y.xx;
}

void out_half3_vh3(out float3 _65)
{
    _65 = _33_colorWhite.z.xxx;
}

void out_half4_vh4(out float4 _73)
{
    _73 = _33_colorWhite.w.xxxx;
}

void out_half2x2_vh22(out float2x2 _82)
{
    _82 = float2x2(float2(_33_colorWhite.x, 0.0f), float2(0.0f, _33_colorWhite.x));
}

void out_half3x3_vh33(out float3x3 _93)
{
    _93 = float3x3(float3(_33_colorWhite.y, 0.0f, 0.0f), float3(0.0f, _33_colorWhite.y, 0.0f), float3(0.0f, 0.0f, _33_colorWhite.y));
}

void out_half4x4_vh44(out float4x4 _105)
{
    _105 = float4x4(float4(_33_colorWhite.z, 0.0f, 0.0f, 0.0f), float4(0.0f, _33_colorWhite.z, 0.0f, 0.0f), float4(0.0f, 0.0f, _33_colorWhite.z, 0.0f), float4(0.0f, 0.0f, 0.0f, _33_colorWhite.z));
}

void out_int_vi(out int _117)
{
    _117 = int(_33_colorWhite.x);
}

void out_int2_vi2(out int2 _126)
{
    _126 = int(_33_colorWhite.y).xx;
}

void out_int3_vi3(out int3 _136)
{
    _136 = int(_33_colorWhite.z).xxx;
}

void out_int4_vi4(out int4 _146)
{
    _146 = int(_33_colorWhite.w).xxxx;
}

void out_float_vf(out float _153)
{
    _153 = _33_colorWhite.x;
}

void out_float2_vf2(out float2 _158)
{
    _158 = _33_colorWhite.y.xx;
}

void out_float3_vf3(out float3 _164)
{
    _164 = _33_colorWhite.z.xxx;
}

void out_float4_vf4(out float4 _170)
{
    _170 = _33_colorWhite.w.xxxx;
}

void out_float2x2_vf22(out float2x2 _176)
{
    _176 = float2x2(float2(_33_colorWhite.x, 0.0f), float2(0.0f, _33_colorWhite.x));
}

void out_float3x3_vf33(out float3x3 _184)
{
    _184 = float3x3(float3(_33_colorWhite.y, 0.0f, 0.0f), float3(0.0f, _33_colorWhite.y, 0.0f), float3(0.0f, 0.0f, _33_colorWhite.y));
}

void out_float4x4_vf44(out float4x4 _193)
{
    _193 = float4x4(float4(_33_colorWhite.z, 0.0f, 0.0f, 0.0f), float4(0.0f, _33_colorWhite.z, 0.0f, 0.0f), float4(0.0f, 0.0f, _33_colorWhite.z, 0.0f), float4(0.0f, 0.0f, 0.0f, _33_colorWhite.z));
}

void out_bool_vb(out bool _206)
{
    _206 = _33_colorWhite.x != 0.0f;
}

void out_bool2_vb2(out bool2 _215)
{
    _215 = (_33_colorWhite.y != 0.0f).xx;
}

void out_bool3_vb3(out bool3 _225)
{
    _225 = (_33_colorWhite.z != 0.0f).xxx;
}

void out_bool4_vb4(out bool4 _235)
{
    _235 = (_33_colorWhite.w != 0.0f).xxxx;
}

float4 main(float2 _243)
{
    float _246 = 0.0f;
    out_half_vh(_246);
    float h = _246;
    float2 _250 = 0.0f.xx;
    out_half2_vh2(_250);
    float2 h2 = _250;
    float3 _254 = 0.0f.xxx;
    out_half3_vh3(_254);
    float3 h3 = _254;
    float4 _258 = 0.0f.xxxx;
    out_half4_vh4(_258);
    float4 h4 = _258;
    float _263 = 0.0f;
    out_half_vh(_263);
    h3.y = _263;
    float2 _266 = 0.0f.xx;
    out_half2_vh2(_266);
    h3 = float3(_266.x, h3.y, _266.y);
    float4 _271 = 0.0f.xxxx;
    out_half4_vh4(_271);
    h4 = float4(_271.z, _271.w, _271.x, _271.y);
    float2x2 _277 = float2x2(0.0f.xx, 0.0f.xx);
    out_half2x2_vh22(_277);
    float2x2 h2x2 = _277;
    float3x3 _281 = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    out_half3x3_vh33(_281);
    float3x3 h3x3 = _281;
    float4x4 _285 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    out_half4x4_vh44(_285);
    float4x4 h4x4 = _285;
    float3 _289 = 0.0f.xxx;
    out_half3_vh3(_289);
    h3x3[1] = _289;
    float _295 = 0.0f;
    out_half_vh(_295);
    h4x4[3].w = _295;
    float _301 = 0.0f;
    out_half_vh(_301);
    h2x2[0].x = _301;
    int _305 = 0;
    out_int_vi(_305);
    int i = _305;
    int2 _309 = int2(0, 0);
    out_int2_vi2(_309);
    int2 i2 = _309;
    int3 _313 = int3(0, 0, 0);
    out_int3_vi3(_313);
    int3 i3 = _313;
    int4 _317 = int4(0, 0, 0, 0);
    out_int4_vi4(_317);
    int4 i4 = _317;
    int3 _320 = int3(0, 0, 0);
    out_int3_vi3(_320);
    i4 = int4(_320.x, _320.y, _320.z, i4.w);
    int _326 = 0;
    out_int_vi(_326);
    i2.y = _326;
    float _330 = 0.0f;
    out_float_vf(_330);
    float f = _330;
    float2 _334 = 0.0f.xx;
    out_float2_vf2(_334);
    float2 f2 = _334;
    float3 _338 = 0.0f.xxx;
    out_float3_vf3(_338);
    float3 f3 = _338;
    float4 _342 = 0.0f.xxxx;
    out_float4_vf4(_342);
    float4 f4 = _342;
    float2 _345 = 0.0f.xx;
    out_float2_vf2(_345);
    f3 = float3(_345.x, _345.y, f3.z);
    float _351 = 0.0f;
    out_float_vf(_351);
    f2.x = _351;
    float2x2 _355 = float2x2(0.0f.xx, 0.0f.xx);
    out_float2x2_vf22(_355);
    float2x2 f2x2 = _355;
    float3x3 _359 = float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx);
    out_float3x3_vf33(_359);
    float3x3 f3x3 = _359;
    float4x4 _363 = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);
    out_float4x4_vf44(_363);
    float4x4 f4x4 = _363;
    float _368 = 0.0f;
    out_float_vf(_368);
    f2x2[0].x = _368;
    bool _372 = false;
    out_bool_vb(_372);
    bool b = _372;
    bool2 _376 = bool2(false, false);
    out_bool2_vb2(_376);
    bool2 b2 = _376;
    bool3 _380 = bool3(false, false, false);
    out_bool3_vb3(_380);
    bool3 b3 = _380;
    bool4 _384 = bool4(false, false, false, false);
    out_bool4_vb4(_384);
    bool4 b4 = _384;
    bool2 _387 = bool2(false, false);
    out_bool2_vb2(_387);
    b4 = bool4(_387.x, b4.y, b4.z, _387.y);
    bool _393 = false;
    out_bool_vb(_393);
    b3.z = _393;
    bool ok = true;
    bool _425 = false;
    if (true)
    {
        _425 = 1.0f == ((((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x);
    }
    else
    {
        _425 = false;
    }
    ok = _425;
    bool _451 = false;
    if (_425)
    {
        _451 = 1.0f == ((((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x);
    }
    else
    {
        _451 = false;
    }
    ok = _451;
    bool _465 = false;
    if (_451)
    {
        _465 = 1 == (((i * i2.x) * i3.x) * i4.x);
    }
    else
    {
        _465 = false;
    }
    ok = _465;
    bool _484 = false;
    if (_465)
    {
        bool _473 = false;
        if (b)
        {
            _473 = b2.x;
        }
        else
        {
            _473 = false;
        }
        bool _478 = false;
        if (_473)
        {
            _478 = b3.x;
        }
        else
        {
            _478 = false;
        }
        bool _483 = false;
        if (_478)
        {
            _483 = b4.x;
        }
        else
        {
            _483 = false;
        }
        _484 = _483;
    }
    else
    {
        _484 = false;
    }
    ok = _484;
    float4 _485 = 0.0f.xxxx;
    if (_484)
    {
        _485 = _33_colorGreen;
    }
    else
    {
        _485 = _33_colorRed;
    }
    return _485;
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
