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
    float4 expected = float4(0.0f, 0.0f, 0.75f, 1.0f);
    bool _55 = false;
    if (clamp(_10_testInputs.x, 0.0f, 1.0f) == expected.x)
    {
        float2 _44 = clamp(_10_testInputs.xy, 0.0f.xx, 1.0f.xx);
        _55 = all(bool2(_44.x == expected.xy.x, _44.y == expected.xy.y));
    }
    else
    {
        _55 = false;
    }
    bool _70 = false;
    if (_55)
    {
        float3 _58 = clamp(_10_testInputs.xyz, 0.0f.xxx, 1.0f.xxx);
        _70 = all(bool3(_58.x == expected.xyz.x, _58.y == expected.xyz.y, _58.z == expected.xyz.z));
    }
    else
    {
        _70 = false;
    }
    bool _82 = false;
    if (_70)
    {
        float4 _73 = clamp(_10_testInputs, 0.0f.xxxx, 1.0f.xxxx);
        _82 = all(bool4(_73.x == expected.x, _73.y == expected.y, _73.z == expected.z, _73.w == expected.w));
    }
    else
    {
        _82 = false;
    }
    bool _88 = false;
    if (_82)
    {
        _88 = 0.0f == expected.x;
    }
    else
    {
        _88 = false;
    }
    bool _95 = false;
    if (_88)
    {
        _95 = all(bool2(0.0f.xx.x == expected.xy.x, 0.0f.xx.y == expected.xy.y));
    }
    else
    {
        _95 = false;
    }
    bool _103 = false;
    if (_95)
    {
        _103 = all(bool3(float3(0.0f, 0.0f, 0.75f).x == expected.xyz.x, float3(0.0f, 0.0f, 0.75f).y == expected.xyz.y, float3(0.0f, 0.0f, 0.75f).z == expected.xyz.z));
    }
    else
    {
        _103 = false;
    }
    bool _109 = false;
    if (_103)
    {
        _109 = all(bool4(float4(0.0f, 0.0f, 0.75f, 1.0f).x == expected.x, float4(0.0f, 0.0f, 0.75f, 1.0f).y == expected.y, float4(0.0f, 0.0f, 0.75f, 1.0f).z == expected.z, float4(0.0f, 0.0f, 0.75f, 1.0f).w == expected.w));
    }
    else
    {
        _109 = false;
    }
    float4 _110 = 0.0f.xxxx;
    if (_109)
    {
        _110 = _10_colorGreen;
    }
    else
    {
        _110 = _10_colorRed;
    }
    return _110;
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
