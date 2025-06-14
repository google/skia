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
    if (ddx(_11_testInputs.x) == 0.0f)
    {
        float2 _41 = ddx(_11_testInputs.xy);
        _49 = all(bool2(_41.x == 0.0f.xxxx.xy.x, _41.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _61 = false;
    if (_49)
    {
        float3 _52 = ddx(_11_testInputs.xyz);
        _61 = all(bool3(_52.x == 0.0f.xxxx.xyz.x, _52.y == 0.0f.xxxx.xyz.y, _52.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _61 = false;
    }
    bool _70 = false;
    if (_61)
    {
        float4 _64 = ddx(_11_testInputs);
        _70 = all(bool4(_64.x == 0.0f.xxxx.x, _64.y == 0.0f.xxxx.y, _64.z == 0.0f.xxxx.z, _64.w == 0.0f.xxxx.w));
    }
    else
    {
        _70 = false;
    }
    bool _81 = false;
    if (_70)
    {
        float2 _73 = sign(ddx(_25.xx));
        _81 = all(bool2(_73.x == 1.0f.xx.x, _73.y == 1.0f.xx.y));
    }
    else
    {
        _81 = false;
    }
    bool _90 = false;
    if (_81)
    {
        float2 _84 = sign(ddx(_25.yy));
        _90 = all(bool2(_84.x == 0.0f.xx.x, _84.y == 0.0f.xx.y));
    }
    else
    {
        _90 = false;
    }
    bool _99 = false;
    if (_90)
    {
        float2 _93 = sign(ddx(_25));
        _99 = all(bool2(_93.x == float2(1.0f, 0.0f).x, _93.y == float2(1.0f, 0.0f).y));
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
