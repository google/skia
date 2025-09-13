cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_testInputs : packoffset(c0);
    float4 _12_colorGreen : packoffset(c1);
    float4 _12_colorRed : packoffset(c2);
    float2 _12_u_skRTFlip : packoffset(c1024);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    float4 expected = 0.0f.xxxx;
    bool _60 = false;
    if ((ddy(_12_testInputs.x) * _12_u_skRTFlip.y) == 0.0f)
    {
        float2 _55 = ddy(_12_testInputs.xy) * _12_u_skRTFlip.yy;
        _60 = all(bool2(_55.x == 0.0f.xxxx.xy.x, _55.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _60 = false;
    }
    bool _76 = false;
    if (_60)
    {
        float3 _71 = ddy(_12_testInputs.xyz) * _12_u_skRTFlip.yyy;
        _76 = all(bool3(_71.x == 0.0f.xxxx.xyz.x, _71.y == 0.0f.xxxx.xyz.y, _71.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _76 = false;
    }
    bool _89 = false;
    if (_76)
    {
        float4 _85 = ddy(_12_testInputs) * _12_u_skRTFlip.yyyy;
        _89 = all(bool4(_85.x == 0.0f.xxxx.x, _85.y == 0.0f.xxxx.y, _85.z == 0.0f.xxxx.z, _85.w == 0.0f.xxxx.w));
    }
    else
    {
        _89 = false;
    }
    bool _102 = false;
    if (_89)
    {
        float2 _92 = sign(ddy(_26.xx) * _12_u_skRTFlip.yy);
        _102 = all(bool2(_92.x == 0.0f.xx.x, _92.y == 0.0f.xx.y));
    }
    else
    {
        _102 = false;
    }
    bool _117 = false;
    if (_102)
    {
        float2 _105 = sign(ddy(_26.yy) * _12_u_skRTFlip.yy);
        _117 = all(bool2(_105.x == 1.0f.xx.x, _105.y == 1.0f.xx.y));
    }
    else
    {
        _117 = false;
    }
    bool _130 = false;
    if (_117)
    {
        float2 _120 = sign(ddy(_26) * _12_u_skRTFlip.yy);
        _130 = all(bool2(_120.x == float2(0.0f, 1.0f).x, _120.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _130 = false;
    }
    float4 _131 = 0.0f.xxxx;
    if (_130)
    {
        _131 = _12_colorGreen;
    }
    else
    {
        _131 = _12_colorRed;
    }
    return _131;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
