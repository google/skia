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
    bool _59 = false;
    if ((ddy(_11_testInputs.x) * _11_u_skRTFlip.y) == 0.0f)
    {
        float2 _54 = ddy(_11_testInputs.xy) * _11_u_skRTFlip.yy;
        _59 = all(bool2(_54.x == 0.0f.xxxx.xy.x, _54.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _59 = false;
    }
    bool _75 = false;
    if (_59)
    {
        float3 _70 = ddy(_11_testInputs.xyz) * _11_u_skRTFlip.yyy;
        _75 = all(bool3(_70.x == 0.0f.xxxx.xyz.x, _70.y == 0.0f.xxxx.xyz.y, _70.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _75 = false;
    }
    bool _88 = false;
    if (_75)
    {
        float4 _84 = ddy(_11_testInputs) * _11_u_skRTFlip.yyyy;
        _88 = all(bool4(_84.x == 0.0f.xxxx.x, _84.y == 0.0f.xxxx.y, _84.z == 0.0f.xxxx.z, _84.w == 0.0f.xxxx.w));
    }
    else
    {
        _88 = false;
    }
    bool _101 = false;
    if (_88)
    {
        float2 _91 = sign(ddy(_25.xx) * _11_u_skRTFlip.yy);
        _101 = all(bool2(_91.x == 0.0f.xx.x, _91.y == 0.0f.xx.y));
    }
    else
    {
        _101 = false;
    }
    bool _116 = false;
    if (_101)
    {
        float2 _104 = sign(ddy(_25.yy) * _11_u_skRTFlip.yy);
        _116 = all(bool2(_104.x == 1.0f.xx.x, _104.y == 1.0f.xx.y));
    }
    else
    {
        _116 = false;
    }
    bool _129 = false;
    if (_116)
    {
        float2 _119 = sign(ddy(_25) * _11_u_skRTFlip.yy);
        _129 = all(bool2(_119.x == float2(0.0f, 1.0f).x, _119.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _129 = false;
    }
    float4 _130 = 0.0f.xxxx;
    if (_129)
    {
        _130 = _11_colorGreen;
    }
    else
    {
        _130 = _11_colorRed;
    }
    return _130;
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
