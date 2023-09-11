cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
    float4 _7_colorWhite : packoffset(c3);
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

float4 main(float2 _21)
{
    float4 expectedA = float4(0.75f, 0.0f, 0.75f, 0.25f);
    float4 expectedB = float4(0.25f, 0.0f, 0.75f, 1.0f);
    bool _52 = false;
    if (mod(_7_testInputs.x, 1.0f) == 0.75f)
    {
        float2 _43 = mod(_7_testInputs.xy, 1.0f.xx);
        _52 = all(bool2(_43.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, _43.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _65 = false;
    if (_52)
    {
        float3 _55 = mod(_7_testInputs.xyz, 1.0f.xxx);
        _65 = all(bool3(_55.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, _55.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, _55.z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
    }
    else
    {
        _65 = false;
    }
    bool _75 = false;
    if (_65)
    {
        float4 _68 = mod(_7_testInputs, 1.0f.xxxx);
        _75 = all(bool4(_68.x == float4(0.75f, 0.0f, 0.75f, 0.25f).x, _68.y == float4(0.75f, 0.0f, 0.75f, 0.25f).y, _68.z == float4(0.75f, 0.0f, 0.75f, 0.25f).z, _68.w == float4(0.75f, 0.0f, 0.75f, 0.25f).w));
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
        _86 = all(bool2(float2(0.75f, 0.0f).x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, float2(0.75f, 0.0f).y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _86 = false;
    }
    bool _93 = false;
    if (_86)
    {
        _93 = all(bool3(float3(0.75f, 0.0f, 0.75f).x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, float3(0.75f, 0.0f, 0.75f).y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, float3(0.75f, 0.0f, 0.75f).z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
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
        _108 = mod(_7_testInputs.x, _7_colorWhite.x) == 0.75f;
    }
    else
    {
        _108 = false;
    }
    bool _121 = false;
    if (_108)
    {
        float2 _111 = mod(_7_testInputs.xy, _7_colorWhite.xy);
        _121 = all(bool2(_111.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, _111.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _121 = false;
    }
    bool _134 = false;
    if (_121)
    {
        float3 _124 = mod(_7_testInputs.xyz, _7_colorWhite.xyz);
        _134 = all(bool3(_124.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, _124.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, _124.z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
    }
    else
    {
        _134 = false;
    }
    bool _144 = false;
    if (_134)
    {
        float4 _137 = mod(_7_testInputs, _7_colorWhite);
        _144 = all(bool4(_137.x == float4(0.75f, 0.0f, 0.75f, 0.25f).x, _137.y == float4(0.75f, 0.0f, 0.75f, 0.25f).y, _137.z == float4(0.75f, 0.0f, 0.75f, 0.25f).z, _137.w == float4(0.75f, 0.0f, 0.75f, 0.25f).w));
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
        _154 = all(bool2(float2(0.25f, 0.0f).x == float4(0.25f, 0.0f, 0.75f, 1.0f).xy.x, float2(0.25f, 0.0f).y == float4(0.25f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _154 = false;
    }
    bool _161 = false;
    if (_154)
    {
        _161 = all(bool3(float3(0.25f, 0.0f, 0.75f).x == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.x, float3(0.25f, 0.0f, 0.75f).y == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.y, float3(0.25f, 0.0f, 0.75f).z == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.z));
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
