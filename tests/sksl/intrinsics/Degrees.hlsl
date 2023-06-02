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
    bool _52 = false;
    if (abs(degrees(_10_testInputs.x) - (-71.61972808837890625f)) < 0.0500000007450580596923828125f)
    {
        float2 _43 = abs(degrees(_10_testInputs.xy) - float2(-71.61972808837890625f, 0.0f));
        _52 = all(bool2(_43.x < 0.0500000007450580596923828125f.xx.x, _43.y < 0.0500000007450580596923828125f.xx.y));
    }
    else
    {
        _52 = false;
    }
    bool _68 = false;
    if (_52)
    {
        float3 _57 = abs(degrees(_10_testInputs.xyz) - float3(-71.61972808837890625f, 0.0f, 42.971836090087890625f));
        _68 = all(bool3(_57.x < 0.0500000007450580596923828125f.xxx.x, _57.y < 0.0500000007450580596923828125f.xxx.y, _57.z < 0.0500000007450580596923828125f.xxx.z));
    }
    else
    {
        _68 = false;
    }
    bool _82 = false;
    if (_68)
    {
        float4 _73 = abs(degrees(_10_testInputs) - float4(-71.61972808837890625f, 0.0f, 42.971836090087890625f, 128.9155120849609375f));
        _82 = all(bool4(_73.x < 0.0500000007450580596923828125f.xxxx.x, _73.y < 0.0500000007450580596923828125f.xxxx.y, _73.z < 0.0500000007450580596923828125f.xxxx.z, _73.w < 0.0500000007450580596923828125f.xxxx.w));
    }
    else
    {
        _82 = false;
    }
    bool4 _83 = _82.xxxx;
    return float4(_83.x ? _10_colorGreen.x : _10_colorRed.x, _83.y ? _10_colorGreen.y : _10_colorRed.y, _83.z ? _10_colorGreen.z : _10_colorRed.z, _83.w ? _10_colorGreen.w : _10_colorRed.w);
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
