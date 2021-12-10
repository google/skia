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
    bool _60 = false;
    if ((ddy(_11_testInputs.x) * _11_u_skRTFlip.y) == 0.0f)
    {
        float2 _55 = ddy(_11_testInputs.xy) * _11_u_skRTFlip.y.xx;
        _60 = all(bool2(_55.x == 0.0f.xxxx.xy.x, _55.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _60 = false;
    }
    bool _77 = false;
    if (_60)
    {
        float3 _72 = ddy(_11_testInputs.xyz) * _11_u_skRTFlip.y.xxx;
        _77 = all(bool3(_72.x == 0.0f.xxxx.xyz.x, _72.y == 0.0f.xxxx.xyz.y, _72.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _77 = false;
    }
    bool _91 = false;
    if (_77)
    {
        float4 _87 = ddy(_11_testInputs) * _11_u_skRTFlip.y.xxxx;
        _91 = all(bool4(_87.x == 0.0f.xxxx.x, _87.y == 0.0f.xxxx.y, _87.z == 0.0f.xxxx.z, _87.w == 0.0f.xxxx.w));
    }
    else
    {
        _91 = false;
    }
    bool _105 = false;
    if (_91)
    {
        float2 _94 = sign(ddy(_25.xx) * _11_u_skRTFlip.y.xx);
        _105 = all(bool2(_94.x == 0.0f.xx.x, _94.y == 0.0f.xx.y));
    }
    else
    {
        _105 = false;
    }
    bool _121 = false;
    if (_105)
    {
        float2 _108 = sign(ddy(_25.yy) * _11_u_skRTFlip.y.xx);
        _121 = all(bool2(_108.x == 1.0f.xx.x, _108.y == 1.0f.xx.y));
    }
    else
    {
        _121 = false;
    }
    bool _135 = false;
    if (_121)
    {
        float2 _124 = sign(ddy(_25) * _11_u_skRTFlip.y.xx);
        _135 = all(bool2(_124.x == float2(0.0f, 1.0f).x, _124.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _135 = false;
    }
    float4 _136 = 0.0f.xxxx;
    if (_135)
    {
        _136 = _11_colorGreen;
    }
    else
    {
        _136 = _11_colorRed;
    }
    return _136;
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
