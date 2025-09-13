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
    float4 expected = 0.0f.xxxx;
    bool _49 = false;
    if (ddy(_11_testInputs.x) == 0.0f)
    {
        float2 _41 = ddy(_11_testInputs.xy);
        _49 = all(bool2(_41.x == 0.0f.xxxx.xy.x, _41.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _61 = false;
    if (_49)
    {
        float3 _52 = ddy(_11_testInputs.xyz);
        _61 = all(bool3(_52.x == 0.0f.xxxx.xyz.x, _52.y == 0.0f.xxxx.xyz.y, _52.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _61 = false;
    }
    bool _70 = false;
    if (_61)
    {
        float4 _64 = ddy(_11_testInputs);
        _70 = all(bool4(_64.x == 0.0f.xxxx.x, _64.y == 0.0f.xxxx.y, _64.z == 0.0f.xxxx.z, _64.w == 0.0f.xxxx.w));
    }
    else
    {
        _70 = false;
    }
    bool _79 = false;
    if (_70)
    {
        float2 _73 = sign(ddy(_25.xx));
        _79 = all(bool2(_73.x == 0.0f.xx.x, _73.y == 0.0f.xx.y));
    }
    else
    {
        _79 = false;
    }
    bool _90 = false;
    if (_79)
    {
        float2 _82 = sign(ddy(_25.yy));
        _90 = all(bool2(_82.x == 1.0f.xx.x, _82.y == 1.0f.xx.y));
    }
    else
    {
        _90 = false;
    }
    bool _99 = false;
    if (_90)
    {
        float2 _93 = sign(ddy(_25));
        _99 = all(bool2(_93.x == float2(0.0f, 1.0f).x, _93.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _99 = false;
    }
    float4 _100 = 0.0f.xxxx;
    if (_99)
    {
        _100 = _11_colorGreen;
    }
    else
    {
        _100 = _11_colorRed;
    }
    return _100;
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
