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
    float4 expectedA = float4(0.5f, 0.5f, 0.75f, 2.25f);
    float4 expectedB = float4(0.0f, 1.0f, 0.75f, 2.25f);
    bool _53 = false;
    if (max(_7_testInputs.x, 0.5f) == 0.5f)
    {
        float2 _44 = max(_7_testInputs.xy, 0.5f.xx);
        _53 = all(bool2(_44.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.x, _44.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _53 = false;
    }
    bool _66 = false;
    if (_53)
    {
        float3 _56 = max(_7_testInputs.xyz, 0.5f.xxx);
        _66 = all(bool3(_56.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.x, _56.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.y, _56.z == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _76 = false;
    if (_66)
    {
        float4 _69 = max(_7_testInputs, 0.5f.xxxx);
        _76 = all(bool4(_69.x == float4(0.5f, 0.5f, 0.75f, 2.25f).x, _69.y == float4(0.5f, 0.5f, 0.75f, 2.25f).y, _69.z == float4(0.5f, 0.5f, 0.75f, 2.25f).z, _69.w == float4(0.5f, 0.5f, 0.75f, 2.25f).w));
    }
    else
    {
        _76 = false;
    }
    bool _80 = false;
    if (_76)
    {
        _80 = true;
    }
    else
    {
        _80 = false;
    }
    bool _86 = false;
    if (_80)
    {
        _86 = all(bool2(0.5f.xx.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.x, 0.5f.xx.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _86 = false;
    }
    bool _93 = false;
    if (_86)
    {
        _93 = all(bool3(float3(0.5f, 0.5f, 0.75f).x == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.x, float3(0.5f, 0.5f, 0.75f).y == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.y, float3(0.5f, 0.5f, 0.75f).z == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.z));
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
        _108 = max(_7_testInputs.x, _7_colorGreen.x) == 0.0f;
    }
    else
    {
        _108 = false;
    }
    bool _121 = false;
    if (_108)
    {
        float2 _111 = max(_7_testInputs.xy, _7_colorGreen.xy);
        _121 = all(bool2(_111.x == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.x, _111.y == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _121 = false;
    }
    bool _134 = false;
    if (_121)
    {
        float3 _124 = max(_7_testInputs.xyz, _7_colorGreen.xyz);
        _134 = all(bool3(_124.x == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.x, _124.y == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.y, _124.z == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _134 = false;
    }
    bool _144 = false;
    if (_134)
    {
        float4 _137 = max(_7_testInputs, _7_colorGreen);
        _144 = all(bool4(_137.x == float4(0.0f, 1.0f, 0.75f, 2.25f).x, _137.y == float4(0.0f, 1.0f, 0.75f, 2.25f).y, _137.z == float4(0.0f, 1.0f, 0.75f, 2.25f).z, _137.w == float4(0.0f, 1.0f, 0.75f, 2.25f).w));
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
    bool _154 = false;
    if (_147)
    {
        _154 = all(bool2(float2(0.0f, 1.0f).x == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.x, float2(0.0f, 1.0f).y == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _154 = false;
    }
    bool _161 = false;
    if (_154)
    {
        _161 = all(bool3(float3(0.0f, 1.0f, 0.75f).x == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.x, float3(0.0f, 1.0f, 0.75f).y == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.y, float3(0.0f, 1.0f, 0.75f).z == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _161 = false;
    }
    bool _164 = false;
    if (_161)
    {
        _164 = true;
    }
    else
    {
        _164 = false;
    }
    float4 _165 = 0.0f.xxxx;
    if (_164)
    {
        _165 = _7_colorGreen;
    }
    else
    {
        _165 = _7_colorRed;
    }
    return _165;
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
