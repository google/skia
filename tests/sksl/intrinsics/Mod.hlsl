cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
    float4 _10_colorWhite : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float mod(float x, float y)
{
    return x - y * floor(x / y);
}

float2 mod(float2 x, float2 y)
{
    return x - y * floor(x / y);
}

float3 mod(float3 x, float3 y)
{
    return x - y * floor(x / y);
}

float4 mod(float4 x, float4 y)
{
    return x - y * floor(x / y);
}

float4 main(float2 _24)
{
    float4 expectedA = float4(0.75f, 0.0f, 0.75f, 0.25f);
    float4 expectedB = float4(0.25f, 0.0f, 0.75f, 1.0f);
    bool _54 = false;
    if (mod(_10_testInputs.x, 1.0f) == 0.75f)
    {
        float2 _45 = mod(_10_testInputs.xy, 1.0f.xx);
        _54 = all(bool2(_45.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, _45.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _54 = false;
    }
    bool _67 = false;
    if (_54)
    {
        float3 _57 = mod(_10_testInputs.xyz, 1.0f.xxx);
        _67 = all(bool3(_57.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, _57.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, _57.z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
    }
    else
    {
        _67 = false;
    }
    bool _77 = false;
    if (_67)
    {
        float4 _70 = mod(_10_testInputs, 1.0f.xxxx);
        _77 = all(bool4(_70.x == float4(0.75f, 0.0f, 0.75f, 0.25f).x, _70.y == float4(0.75f, 0.0f, 0.75f, 0.25f).y, _70.z == float4(0.75f, 0.0f, 0.75f, 0.25f).z, _70.w == float4(0.75f, 0.0f, 0.75f, 0.25f).w));
    }
    else
    {
        _77 = false;
    }
    bool _81 = false;
    if (_77)
    {
        _81 = true;
    }
    else
    {
        _81 = false;
    }
    bool _88 = false;
    if (_81)
    {
        _88 = all(bool2(float2(0.75f, 0.0f).x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, float2(0.75f, 0.0f).y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _88 = false;
    }
    bool _95 = false;
    if (_88)
    {
        _95 = all(bool3(float3(0.75f, 0.0f, 0.75f).x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, float3(0.75f, 0.0f, 0.75f).y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, float3(0.75f, 0.0f, 0.75f).z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
    }
    else
    {
        _95 = false;
    }
    bool _98 = false;
    if (_95)
    {
        _98 = true;
    }
    else
    {
        _98 = false;
    }
    bool _110 = false;
    if (_98)
    {
        _110 = mod(_10_testInputs.x, _10_colorWhite.x) == 0.75f;
    }
    else
    {
        _110 = false;
    }
    bool _123 = false;
    if (_110)
    {
        float2 _113 = mod(_10_testInputs.xy, _10_colorWhite.xy);
        _123 = all(bool2(_113.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, _113.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _123 = false;
    }
    bool _136 = false;
    if (_123)
    {
        float3 _126 = mod(_10_testInputs.xyz, _10_colorWhite.xyz);
        _136 = all(bool3(_126.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, _126.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, _126.z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
    }
    else
    {
        _136 = false;
    }
    bool _146 = false;
    if (_136)
    {
        float4 _139 = mod(_10_testInputs, _10_colorWhite);
        _146 = all(bool4(_139.x == float4(0.75f, 0.0f, 0.75f, 0.25f).x, _139.y == float4(0.75f, 0.0f, 0.75f, 0.25f).y, _139.z == float4(0.75f, 0.0f, 0.75f, 0.25f).z, _139.w == float4(0.75f, 0.0f, 0.75f, 0.25f).w));
    }
    else
    {
        _146 = false;
    }
    bool _149 = false;
    if (_146)
    {
        _149 = true;
    }
    else
    {
        _149 = false;
    }
    bool _156 = false;
    if (_149)
    {
        _156 = all(bool2(float2(0.25f, 0.0f).x == float4(0.25f, 0.0f, 0.75f, 1.0f).xy.x, float2(0.25f, 0.0f).y == float4(0.25f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _156 = false;
    }
    bool _163 = false;
    if (_156)
    {
        _163 = all(bool3(float3(0.25f, 0.0f, 0.75f).x == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.x, float3(0.25f, 0.0f, 0.75f).y == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.y, float3(0.25f, 0.0f, 0.75f).z == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _163 = false;
    }
    bool _166 = false;
    if (_163)
    {
        _166 = true;
    }
    else
    {
        _166 = false;
    }
    float4 _167 = 0.0f.xxxx;
    if (_166)
    {
        _167 = _10_colorGreen;
    }
    else
    {
        _167 = _10_colorRed;
    }
    return _167;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
