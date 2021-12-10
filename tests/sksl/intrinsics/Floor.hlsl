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
    float4 expected = float4(-2.0f, 0.0f, 0.0f, 2.0f);
    bool _53 = false;
    if (floor(_10_testInputs.x) == expected.x)
    {
        float2 _44 = floor(_10_testInputs.xy);
        _53 = all(bool2(_44.x == expected.xy.x, _44.y == expected.xy.y));
    }
    else
    {
        _53 = false;
    }
    bool _66 = false;
    if (_53)
    {
        float3 _56 = floor(_10_testInputs.xyz);
        _66 = all(bool3(_56.x == expected.xyz.x, _56.y == expected.xyz.y, _56.z == expected.xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _76 = false;
    if (_66)
    {
        float4 _69 = floor(_10_testInputs);
        _76 = all(bool4(_69.x == expected.x, _69.y == expected.y, _69.z == expected.z, _69.w == expected.w));
    }
    else
    {
        _76 = false;
    }
    bool _82 = false;
    if (_76)
    {
        _82 = (-2.0f) == expected.x;
    }
    else
    {
        _82 = false;
    }
    bool _90 = false;
    if (_82)
    {
        _90 = all(bool2(float2(-2.0f, 0.0f).x == expected.xy.x, float2(-2.0f, 0.0f).y == expected.xy.y));
    }
    else
    {
        _90 = false;
    }
    bool _98 = false;
    if (_90)
    {
        _98 = all(bool3(float3(-2.0f, 0.0f, 0.0f).x == expected.xyz.x, float3(-2.0f, 0.0f, 0.0f).y == expected.xyz.y, float3(-2.0f, 0.0f, 0.0f).z == expected.xyz.z));
    }
    else
    {
        _98 = false;
    }
    bool _104 = false;
    if (_98)
    {
        _104 = all(bool4(float4(-2.0f, 0.0f, 0.0f, 2.0f).x == expected.x, float4(-2.0f, 0.0f, 0.0f, 2.0f).y == expected.y, float4(-2.0f, 0.0f, 0.0f, 2.0f).z == expected.z, float4(-2.0f, 0.0f, 0.0f, 2.0f).w == expected.w));
    }
    else
    {
        _104 = false;
    }
    float4 _105 = 0.0f.xxxx;
    if (_104)
    {
        _105 = _10_colorGreen;
    }
    else
    {
        _105 = _10_colorRed;
    }
    return _105;
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
