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
        float2 _73 = sign(fwidth(_25.xx));
        _81 = all(bool2(_73.x == 1.0f.xx.x, _73.y == 1.0f.xx.y));
    }
    else
    {
        _81 = false;
    }
    bool _92 = false;
    if (_81)
    {
        float2 _84 = sign(fwidth(float2(_25.x, 1.0f)));
        _92 = all(bool2(_84.x == float2(1.0f, 0.0f).x, _84.y == float2(1.0f, 0.0f).y));
    }
    else
    {
        _92 = false;
    }
    bool _101 = false;
    if (_92)
    {
        float2 _95 = sign(fwidth(_25.yy));
        _101 = all(bool2(_95.x == 1.0f.xx.x, _95.y == 1.0f.xx.y));
    }
    else
    {
        _101 = false;
    }
    bool _112 = false;
    if (_101)
    {
        float2 _104 = sign(fwidth(float2(0.0f, _25.y)));
        _112 = all(bool2(_104.x == float2(0.0f, 1.0f).x, _104.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _112 = false;
    }
    bool _120 = false;
    if (_112)
    {
        float2 _115 = sign(fwidth(_25));
        _120 = all(bool2(_115.x == 1.0f.xx.x, _115.y == 1.0f.xx.y));
    }
    else
    {
        _120 = false;
    }
    float4 _121 = 0.0f.xxxx;
    if (_120)
    {
        _121 = _11_colorGreen;
    }
    else
    {
        _121 = _11_colorRed;
    }
    return _121;
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
