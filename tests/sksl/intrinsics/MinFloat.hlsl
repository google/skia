cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 expectedA = float4(-1.25f, 0.0f, 0.5f, 0.5f);
    float4 expectedB = float4(-1.25f, 0.0f, 0.0f, 1.0f);
    bool _54 = false;
    if (min(_10_testInputs.x, 0.5f) == (-1.25f))
    {
        float2 _45 = min(_10_testInputs.xy, 0.5f.xx);
        _54 = all(bool2(_45.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.x, _45.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.y));
    }
    else
    {
        _54 = false;
    }
    bool _67 = false;
    if (_54)
    {
        float3 _57 = min(_10_testInputs.xyz, 0.5f.xxx);
        _67 = all(bool3(_57.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.x, _57.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.y, _57.z == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.z));
    }
    else
    {
        _67 = false;
    }
    bool _77 = false;
    if (_67)
    {
        float4 _70 = min(_10_testInputs, 0.5f.xxxx);
        _77 = all(bool4(_70.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).x, _70.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).y, _70.z == float4(-1.25f, 0.0f, 0.5f, 0.5f).z, _70.w == float4(-1.25f, 0.0f, 0.5f, 0.5f).w));
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
        _88 = all(bool2(float2(-1.25f, 0.0f).x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.x, float2(-1.25f, 0.0f).y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.y));
    }
    else
    {
        _88 = false;
    }
    bool _95 = false;
    if (_88)
    {
        _95 = all(bool3(float3(-1.25f, 0.0f, 0.5f).x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.x, float3(-1.25f, 0.0f, 0.5f).y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.y, float3(-1.25f, 0.0f, 0.5f).z == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.z));
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
        _110 = min(_10_testInputs.x, _10_colorGreen.x) == (-1.25f);
    }
    else
    {
        _110 = false;
    }
    bool _123 = false;
    if (_110)
    {
        float2 _113 = min(_10_testInputs.xy, _10_colorGreen.xy);
        _123 = all(bool2(_113.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.x, _113.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.y));
    }
    else
    {
        _123 = false;
    }
    bool _136 = false;
    if (_123)
    {
        float3 _126 = min(_10_testInputs.xyz, _10_colorGreen.xyz);
        _136 = all(bool3(_126.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.x, _126.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.y, _126.z == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.z));
    }
    else
    {
        _136 = false;
    }
    bool _146 = false;
    if (_136)
    {
        float4 _139 = min(_10_testInputs, _10_colorGreen);
        _146 = all(bool4(_139.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).x, _139.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).y, _139.z == float4(-1.25f, 0.0f, 0.0f, 1.0f).z, _139.w == float4(-1.25f, 0.0f, 0.0f, 1.0f).w));
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
    bool _155 = false;
    if (_149)
    {
        _155 = all(bool2(float2(-1.25f, 0.0f).x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.x, float2(-1.25f, 0.0f).y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.y));
    }
    else
    {
        _155 = false;
    }
    bool _162 = false;
    if (_155)
    {
        _162 = all(bool3(float3(-1.25f, 0.0f, 0.0f).x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.x, float3(-1.25f, 0.0f, 0.0f).y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.y, float3(-1.25f, 0.0f, 0.0f).z == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.z));
    }
    else
    {
        _162 = false;
    }
    bool _165 = false;
    if (_162)
    {
        _165 = true;
    }
    else
    {
        _165 = false;
    }
    float4 _166 = 0.0f.xxxx;
    if (_165)
    {
        _166 = _10_colorGreen;
    }
    else
    {
        _166 = _10_colorRed;
    }
    return _166;
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
