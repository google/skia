cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
    float2 _11_u_skRTFlip : packoffset(c1024);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 expected = 0.0f.xxxx;
    bool _63 = false;
    if ((ddy(_11_testInputs.x) * _11_u_skRTFlip.y) == expected.x)
    {
        float2 _57 = ddy(_11_testInputs.xy) * _11_u_skRTFlip.y.xx;
        _63 = all(bool2(_57.x == expected.xy.x, _57.y == expected.xy.y));
    }
    else
    {
        _63 = false;
    }
    bool _81 = false;
    if (_63)
    {
        float3 _75 = ddy(_11_testInputs.xyz) * _11_u_skRTFlip.y.xxx;
        _81 = all(bool3(_75.x == expected.xyz.x, _75.y == expected.xyz.y, _75.z == expected.xyz.z));
    }
    else
    {
        _81 = false;
    }
    bool _96 = false;
    if (_81)
    {
        float4 _91 = ddy(_11_testInputs) * _11_u_skRTFlip.y.xxxx;
        _96 = all(bool4(_91.x == expected.x, _91.y == expected.y, _91.z == expected.z, _91.w == expected.w));
    }
    else
    {
        _96 = false;
    }
    bool _110 = false;
    if (_96)
    {
        float2 _99 = sign(ddy(_25.xx) * _11_u_skRTFlip.y.xx);
        _110 = all(bool2(_99.x == 0.0f.xx.x, _99.y == 0.0f.xx.y));
    }
    else
    {
        _110 = false;
    }
    bool _126 = false;
    if (_110)
    {
        float2 _113 = sign(ddy(_25.yy) * _11_u_skRTFlip.y.xx);
        _126 = all(bool2(_113.x == 1.0f.xx.x, _113.y == 1.0f.xx.y));
    }
    else
    {
        _126 = false;
    }
    bool _140 = false;
    if (_126)
    {
        float2 _129 = sign(ddy(_25) * _11_u_skRTFlip.y.xx);
        _140 = all(bool2(_129.x == float2(0.0f, 1.0f).x, _129.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _140 = false;
    }
    float4 _141 = 0.0f.xxxx;
    if (_140)
    {
        _141 = _11_colorGreen;
    }
    else
    {
        _141 = _11_colorRed;
    }
    return _141;
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
