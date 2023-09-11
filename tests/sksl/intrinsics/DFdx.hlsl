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
        float2 _70 = sign(ddx(_21.xx));
        _78 = all(bool2(_70.x == 1.0f.xx.x, _70.y == 1.0f.xx.y));
    }
    else
    {
        _78 = false;
    }
    bool _87 = false;
    if (_78)
    {
        float2 _81 = sign(ddx(_21.yy));
        _87 = all(bool2(_81.x == 0.0f.xx.x, _81.y == 0.0f.xx.y));
    }
    else
    {
        _87 = false;
    }
    bool _96 = false;
    if (_87)
    {
        float2 _90 = sign(ddx(_21));
        _96 = all(bool2(_90.x == float2(1.0f, 0.0f).x, _90.y == float2(1.0f, 0.0f).y));
    }
    else
    {
        _96 = false;
    }
    float4 _97 = 0.0f.xxxx;
    if (_96)
    {
        _97 = _7_colorGreen;
    }
    else
    {
        _97 = _7_colorRed;
    }
    return _97;
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
