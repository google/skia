cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 expected = 0.0f.xxxx;
    bool _46 = false;
    if (ddx(_7_testInputs.x) == 0.0f)
    {
        float2 _38 = ddx(_7_testInputs.xy);
        _46 = all(bool2(_38.x == 0.0f.xxxx.xy.x, _38.y == 0.0f.xxxx.xy.y));
    }
    else
    {
        _46 = false;
    }
    bool _58 = false;
    if (_46)
    {
        float3 _49 = ddx(_7_testInputs.xyz);
        _58 = all(bool3(_49.x == 0.0f.xxxx.xyz.x, _49.y == 0.0f.xxxx.xyz.y, _49.z == 0.0f.xxxx.xyz.z));
    }
    else
    {
        _58 = false;
    }
    bool _67 = false;
    if (_58)
    {
        float4 _61 = ddx(_7_testInputs);
        _67 = all(bool4(_61.x == 0.0f.xxxx.x, _61.y == 0.0f.xxxx.y, _61.z == 0.0f.xxxx.z, _61.w == 0.0f.xxxx.w));
    }
    else
    {
        _67 = false;
    }
    bool _78 = false;
    if (_67)
    {
        float2 _70 = sign(fwidth(_21.xx));
        _78 = all(bool2(_70.x == 1.0f.xx.x, _70.y == 1.0f.xx.y));
    }
    else
    {
        _78 = false;
    }
    bool _89 = false;
    if (_78)
    {
        float2 _81 = sign(fwidth(float2(_21.x, 1.0f)));
        _89 = all(bool2(_81.x == float2(1.0f, 0.0f).x, _81.y == float2(1.0f, 0.0f).y));
    }
    else
    {
        _89 = false;
    }
    bool _98 = false;
    if (_89)
    {
        float2 _92 = sign(fwidth(_21.yy));
        _98 = all(bool2(_92.x == 1.0f.xx.x, _92.y == 1.0f.xx.y));
    }
    else
    {
        _98 = false;
    }
    bool _109 = false;
    if (_98)
    {
        float2 _101 = sign(fwidth(float2(0.0f, _21.y)));
        _109 = all(bool2(_101.x == float2(0.0f, 1.0f).x, _101.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _109 = false;
    }
    bool _117 = false;
    if (_109)
    {
        float2 _112 = sign(fwidth(_21));
        _117 = all(bool2(_112.x == 1.0f.xx.x, _112.y == 1.0f.xx.y));
    }
    else
    {
        _117 = false;
    }
    float4 _118 = 0.0f.xxxx;
    if (_117)
    {
        _118 = _7_colorGreen;
    }
    else
    {
        _118 = _7_colorRed;
    }
    return _118;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
