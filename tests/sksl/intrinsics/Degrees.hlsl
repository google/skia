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
    bool _50 = false;
    if (abs(degrees(_7_testInputs.x) - (-71.61972808837890625f)) < 0.0500000007450580596923828125f)
    {
        float2 _41 = abs(degrees(_7_testInputs.xy) - float2(-71.61972808837890625f, 0.0f));
        _50 = all(bool2(_41.x < 0.0500000007450580596923828125f.xx.x, _41.y < 0.0500000007450580596923828125f.xx.y));
    }
    else
    {
        _50 = false;
    }
    bool _66 = false;
    if (_50)
    {
        float3 _55 = abs(degrees(_7_testInputs.xyz) - float3(-71.61972808837890625f, 0.0f, 42.971836090087890625f));
        _66 = all(bool3(_55.x < 0.0500000007450580596923828125f.xxx.x, _55.y < 0.0500000007450580596923828125f.xxx.y, _55.z < 0.0500000007450580596923828125f.xxx.z));
    }
    else
    {
        _66 = false;
    }
    bool _80 = false;
    if (_66)
    {
        float4 _71 = abs(degrees(_7_testInputs) - float4(-71.61972808837890625f, 0.0f, 42.971836090087890625f, 128.9155120849609375f));
        _80 = all(bool4(_71.x < 0.0500000007450580596923828125f.xxxx.x, _71.y < 0.0500000007450580596923828125f.xxxx.y, _71.z < 0.0500000007450580596923828125f.xxxx.z, _71.w < 0.0500000007450580596923828125f.xxxx.w));
    }
    else
    {
        _80 = false;
    }
    float4 _81 = 0.0f.xxxx;
    if (_80)
    {
        _81 = _7_colorGreen;
    }
    else
    {
        _81 = _7_colorRed;
    }
    return _81;
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
