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
    float4 expected = 0.0f.xxxx;
    bool _48 = false;
    if (ddx(_10_testInputs.x) == 0.0f)
    {
        float2 _40 = ddx(_10_testInputs.xy);
        _48 = all(bool2(_40.x == 0.0f.xxxx.xy.x, _40.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _48 = false;
    }
    bool _60 = false;
    if (_48)
    {
        float3 _51 = ddx(_10_testInputs.xyz);
        _60 = all(bool3(_51.x == 0.0f.xxxx.xyz.x, _51.y == 0.0f.xxxx.xyz.y, _51.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _60 = false;
    }
    bool _69 = false;
    if (_60)
    {
        float4 _63 = ddx(_10_testInputs);
        _69 = all(bool4(_63.x == 0.0f.xxxx.x, _63.y == 0.0f.xxxx.y, _63.z == 0.0f.xxxx.z, _63.w == 0.0f.xxxx.w));
    }
    else
    {
        _69 = false;
    }
    bool _80 = false;
    if (_69)
    {
        float2 _72 = sign(ddx(_24.xx));
        _80 = all(bool2(_72.x == 1.0f.xx.x, _72.y == 1.0f.xx.y));
    }
    else
    {
        _80 = false;
    }
    bool _89 = false;
    if (_80)
    {
        float2 _83 = sign(ddx(_24.yy));
        _89 = all(bool2(_83.x == 0.0f.xx.x, _83.y == 0.0f.xx.y));
    }
    else
    {
        _89 = false;
    }
    bool _98 = false;
    if (_89)
    {
        float2 _92 = sign(ddx(_24));
        _98 = all(bool2(_92.x == float2(1.0f, 0.0f).x, _92.y == float2(1.0f, 0.0f).y));
    }
    else
    {
        _98 = false;
    }
    float4 _99 = 0.0f.xxxx;
    if (_98)
    {
        _99 = _10_colorGreen;
    }
    else
    {
        _99 = _10_colorRed;
    }
    return _99;
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
