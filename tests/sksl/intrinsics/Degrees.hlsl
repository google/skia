cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool _53 = false;
    if (abs(degrees(_11_testInputs.x) - (-71.61972808837890625f)) < 0.0500000007450580596923828125f)
    {
        float2 _44 = abs(degrees(_11_testInputs.xy) - float2(-71.61972808837890625f, 0.0f));
        _53 = all(bool2(_44.x < 0.0500000007450580596923828125f.xx.x, _44.y < 0.0500000007450580596923828125f.xx.y));
    }
    else
    {
        _53 = false;
    }
    bool _69 = false;
    if (_53)
    {
        float3 _58 = abs(degrees(_11_testInputs.xyz) - float3(-71.61972808837890625f, 0.0f, 42.971836090087890625f));
        _69 = all(bool3(_58.x < 0.0500000007450580596923828125f.xxx.x, _58.y < 0.0500000007450580596923828125f.xxx.y, _58.z < 0.0500000007450580596923828125f.xxx.z));
    }
    else
    {
        _69 = false;
    }
    bool _83 = false;
    if (_69)
    {
        float4 _74 = abs(degrees(_11_testInputs) - float4(-71.61972808837890625f, 0.0f, 42.971836090087890625f, 128.9155120849609375f));
        _83 = all(bool4(_74.x < 0.0500000007450580596923828125f.xxxx.x, _74.y < 0.0500000007450580596923828125f.xxxx.y, _74.z < 0.0500000007450580596923828125f.xxxx.z, _74.w < 0.0500000007450580596923828125f.xxxx.w));
    }
    else
    {
        _83 = false;
    }
    float4 _84 = 0.0f.xxxx;
    if (_83)
    {
        _84 = _11_colorGreen;
    }
    else
    {
        _84 = _11_colorRed;
    }
    return _84;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
