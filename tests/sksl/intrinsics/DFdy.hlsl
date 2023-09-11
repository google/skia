cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_testInputs : packoffset(c0);
    float4 _8_colorGreen : packoffset(c1);
    float4 _8_colorRed : packoffset(c2);
    float2 _8_u_skRTFlip : packoffset(c1024);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    float4 expected = 0.0f.xxxx;
    bool _57 = false;
    if ((ddy(_8_testInputs.x) * _8_u_skRTFlip.y) == 0.0f)
    {
        float2 _52 = ddy(_8_testInputs.xy) * _8_u_skRTFlip.yy;
        _57 = all(bool2(_52.x == 0.0f.xxxx.xy.x, _52.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _57 = false;
    }
    bool _73 = false;
    if (_57)
    {
        float3 _68 = ddy(_8_testInputs.xyz) * _8_u_skRTFlip.yyy;
        _73 = all(bool3(_68.x == 0.0f.xxxx.xyz.x, _68.y == 0.0f.xxxx.xyz.y, _68.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _73 = false;
    }
    bool _86 = false;
    if (_73)
    {
        float4 _82 = ddy(_8_testInputs) * _8_u_skRTFlip.yyyy;
        _86 = all(bool4(_82.x == 0.0f.xxxx.x, _82.y == 0.0f.xxxx.y, _82.z == 0.0f.xxxx.z, _82.w == 0.0f.xxxx.w));
    }
    else
    {
        _86 = false;
    }
    bool _99 = false;
    if (_86)
    {
        float2 _89 = sign(ddy(_22.xx) * _8_u_skRTFlip.yy);
        _99 = all(bool2(_89.x == 0.0f.xx.x, _89.y == 0.0f.xx.y));
    }
    else
    {
        _99 = false;
    }
    bool _114 = false;
    if (_99)
    {
        float2 _102 = sign(ddy(_22.yy) * _8_u_skRTFlip.yy);
        _114 = all(bool2(_102.x == 1.0f.xx.x, _102.y == 1.0f.xx.y));
    }
    else
    {
        _114 = false;
    }
    bool _127 = false;
    if (_114)
    {
        float2 _117 = sign(ddy(_22) * _8_u_skRTFlip.yy);
        _127 = all(bool2(_117.x == float2(0.0f, 1.0f).x, _117.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _127 = false;
    }
    float4 _128 = 0.0f.xxxx;
    if (_127)
    {
        _128 = _8_colorGreen;
    }
    else
    {
        _128 = _8_colorRed;
    }
    return _128;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
