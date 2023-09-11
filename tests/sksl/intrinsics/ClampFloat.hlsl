cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 expectedA = float4(-1.0f, 0.0f, 0.75f, 1.0f);
    float4 expectedB = float4(-1.0f, 0.0f, 0.5f, 2.25f);
    bool _55 = false;
    if (clamp(_7_testInputs.x, -1.0f, 1.0f) == (-1.0f))
    {
        float2 _45 = clamp(_7_testInputs.xy, (-1.0f).xx, 1.0f.xx);
        _55 = all(bool2(_45.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.x, _45.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _55 = false;
    }
    bool _69 = false;
    if (_55)
    {
        float3 _58 = clamp(_7_testInputs.xyz, (-1.0f).xxx, 1.0f.xxx);
        _69 = all(bool3(_58.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.x, _58.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.y, _58.z == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _69 = false;
    }
    bool _80 = false;
    if (_69)
    {
        float4 _72 = clamp(_7_testInputs, (-1.0f).xxxx, 1.0f.xxxx);
        _80 = all(bool4(_72.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).x, _72.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).y, _72.z == float4(-1.0f, 0.0f, 0.75f, 1.0f).z, _72.w == float4(-1.0f, 0.0f, 0.75f, 1.0f).w));
    }
    else
    {
        _80 = false;
    }
    bool _88 = false;
    if (_80)
    {
        _88 = clamp(_7_testInputs.x, -1.0f, 1.0f) == (-1.0f);
    }
    else
    {
        _88 = false;
    }
    bool _102 = false;
    if (_88)
    {
        float2 _91 = clamp(_7_testInputs.xy, float2(-1.0f, -2.0f), float2(1.0f, 2.0f));
        _102 = all(bool2(_91.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.x, _91.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.y));
    }
    else
    {
        _102 = false;
    }
    bool _114 = false;
    if (_102)
    {
        float3 _105 = clamp(_7_testInputs.xyz, float3(-1.0f, -2.0f, -2.0f), float3(1.0f, 2.0f, 0.5f));
        _114 = all(bool3(_105.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.x, _105.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.y, _105.z == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.z));
    }
    else
    {
        _114 = false;
    }
    bool _125 = false;
    if (_114)
    {
        float4 _117 = clamp(_7_testInputs, float4(-1.0f, -2.0f, -2.0f, 1.0f), float4(1.0f, 2.0f, 0.5f, 3.0f));
        _125 = all(bool4(_117.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).x, _117.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).y, _117.z == float4(-1.0f, 0.0f, 0.5f, 2.25f).z, _117.w == float4(-1.0f, 0.0f, 0.5f, 2.25f).w));
    }
    else
    {
        _125 = false;
    }
    bool _129 = false;
    if (_125)
    {
        _129 = true;
    }
    else
    {
        _129 = false;
    }
    bool _136 = false;
    if (_129)
    {
        _136 = all(bool2(float2(-1.0f, 0.0f).x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.x, float2(-1.0f, 0.0f).y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _136 = false;
    }
    bool _143 = false;
    if (_136)
    {
        _143 = all(bool3(float3(-1.0f, 0.0f, 0.75f).x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.x, float3(-1.0f, 0.0f, 0.75f).y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.y, float3(-1.0f, 0.0f, 0.75f).z == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _143 = false;
    }
    bool _146 = false;
    if (_143)
    {
        _146 = true;
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
        _155 = all(bool2(float2(-1.0f, 0.0f).x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.x, float2(-1.0f, 0.0f).y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.y));
    }
    else
    {
        _155 = false;
    }
    bool _162 = false;
    if (_155)
    {
        _162 = all(bool3(float3(-1.0f, 0.0f, 0.5f).x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.x, float3(-1.0f, 0.0f, 0.5f).y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.y, float3(-1.0f, 0.0f, 0.5f).z == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.z));
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
        _166 = _7_colorGreen;
    }
    else
    {
        _166 = _7_colorRed;
    }
    return _166;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
