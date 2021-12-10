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
    float4 expected = float4(-1.5625f, 0.0f, 0.75f, 3.375f);
    float4 exponents = float4(2.0f, 3.0f, 1.0f, 1.5f);
    bool _61 = false;
    if (pow(_10_testInputs.x, 2.0f) == expected.x)
    {
        float2 _51 = pow(_10_testInputs.xy, float2(2.0f, 3.0f));
        _61 = all(bool2(_51.x == expected.xy.x, _51.y == expected.xy.y));
    }
    else
    {
        _61 = false;
    }
    bool _75 = false;
    if (_61)
    {
        float3 _64 = pow(_10_testInputs.xyz, float3(2.0f, 3.0f, 1.0f));
        _75 = all(bool3(_64.x == expected.xyz.x, _64.y == expected.xyz.y, _64.z == expected.xyz.z));
    }
    else
    {
        _75 = false;
    }
    bool _86 = false;
    if (_75)
    {
        float4 _78 = pow(_10_testInputs, exponents);
        _86 = all(bool4(_78.x == expected.x, _78.y == expected.y, _78.z == expected.z, _78.w == expected.w));
    }
    else
    {
        _86 = false;
    }
    bool _93 = false;
    if (_86)
    {
        _93 = 1.5625f == expected.x;
    }
    else
    {
        _93 = false;
    }
    bool _101 = false;
    if (_93)
    {
        _101 = all(bool2(float2(1.5625f, 0.0f).x == expected.xy.x, float2(1.5625f, 0.0f).y == expected.xy.y));
    }
    else
    {
        _101 = false;
    }
    bool _109 = false;
    if (_101)
    {
        _109 = all(bool3(float3(1.5625f, 0.0f, 0.75f).x == expected.xyz.x, float3(1.5625f, 0.0f, 0.75f).y == expected.xyz.y, float3(1.5625f, 0.0f, 0.75f).z == expected.xyz.z));
    }
    else
    {
        _109 = false;
    }
    bool _116 = false;
    if (_109)
    {
        _116 = all(bool4(float4(1.5625f, 0.0f, 0.75f, 3.375f).x == expected.x, float4(1.5625f, 0.0f, 0.75f, 3.375f).y == expected.y, float4(1.5625f, 0.0f, 0.75f, 3.375f).z == expected.z, float4(1.5625f, 0.0f, 0.75f, 3.375f).w == expected.w));
    }
    else
    {
        _116 = false;
    }
    float4 _117 = 0.0f.xxxx;
    if (_116)
    {
        _117 = _10_colorGreen;
    }
    else
    {
        _117 = _10_colorRed;
    }
    return _117;
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
