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
    float4 expectedA = float4(-1.25f, 0.0f, 0.5f, 0.5f);
    float4 expectedB = float4(-1.25f, 0.0f, 0.0f, 1.0f);
    bool _52 = false;
    if (min(_7_testInputs.x, 0.5f) == (-1.25f))
    {
        float2 _43 = min(_7_testInputs.xy, 0.5f.xx);
        _52 = all(bool2(_43.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.x, _43.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _65 = false;
    if (_52)
    {
        float3 _55 = min(_7_testInputs.xyz, 0.5f.xxx);
        _65 = all(bool3(_55.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.x, _55.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.y, _55.z == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.z));
    }
    else
    {
        _65 = false;
    }
    bool _75 = false;
    if (_65)
    {
        float4 _68 = min(_7_testInputs, 0.5f.xxxx);
        _75 = all(bool4(_68.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).x, _68.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).y, _68.z == float4(-1.25f, 0.0f, 0.5f, 0.5f).z, _68.w == float4(-1.25f, 0.0f, 0.5f, 0.5f).w));
    }
    else
    {
        _75 = false;
    }
    bool _79 = false;
    if (_75)
    {
        _79 = true;
    }
    else
    {
        _79 = false;
    }
    bool _86 = false;
    if (_79)
    {
        _86 = all(bool2(float2(-1.25f, 0.0f).x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.x, float2(-1.25f, 0.0f).y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.y));
    }
    else
    {
        _86 = false;
    }
    bool _93 = false;
    if (_86)
    {
        _93 = all(bool3(float3(-1.25f, 0.0f, 0.5f).x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.x, float3(-1.25f, 0.0f, 0.5f).y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.y, float3(-1.25f, 0.0f, 0.5f).z == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.z));
    }
    else
    {
        _93 = false;
    }
    bool _96 = false;
    if (_93)
    {
        _96 = true;
    }
    else
    {
        _96 = false;
    }
    bool _108 = false;
    if (_96)
    {
        _108 = min(_7_testInputs.x, _7_colorGreen.x) == (-1.25f);
    }
    else
    {
        _108 = false;
    }
    bool _121 = false;
    if (_108)
    {
        float2 _111 = min(_7_testInputs.xy, _7_colorGreen.xy);
        _121 = all(bool2(_111.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.x, _111.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.y));
    }
    else
    {
        _121 = false;
    }
    bool _134 = false;
    if (_121)
    {
        float3 _124 = min(_7_testInputs.xyz, _7_colorGreen.xyz);
        _134 = all(bool3(_124.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.x, _124.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.y, _124.z == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.z));
    }
    else
    {
        _134 = false;
    }
    bool _144 = false;
    if (_134)
    {
        float4 _137 = min(_7_testInputs, _7_colorGreen);
        _144 = all(bool4(_137.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).x, _137.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).y, _137.z == float4(-1.25f, 0.0f, 0.0f, 1.0f).z, _137.w == float4(-1.25f, 0.0f, 0.0f, 1.0f).w));
    }
    else
    {
        _144 = false;
    }
    bool _147 = false;
    if (_144)
    {
        _147 = true;
    }
    else
    {
        _147 = false;
    }
    bool _153 = false;
    if (_147)
    {
        _153 = all(bool2(float2(-1.25f, 0.0f).x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.x, float2(-1.25f, 0.0f).y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.y));
    }
    else
    {
        _153 = false;
    }
    bool _160 = false;
    if (_153)
    {
        _160 = all(bool3(float3(-1.25f, 0.0f, 0.0f).x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.x, float3(-1.25f, 0.0f, 0.0f).y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.y, float3(-1.25f, 0.0f, 0.0f).z == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.z));
    }
    else
    {
        _160 = false;
    }
    bool _163 = false;
    if (_160)
    {
        _163 = true;
    }
    else
    {
        _163 = false;
    }
    float4 _164 = 0.0f.xxxx;
    if (_163)
    {
        _164 = _7_colorGreen;
    }
    else
    {
        _164 = _7_colorRed;
    }
    return _164;
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
