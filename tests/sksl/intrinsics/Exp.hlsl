cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_inputVal : packoffset(c0);
    float4 _7_expected : packoffset(c1);
    float4 _7_colorGreen : packoffset(c2);
    float4 _7_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool _49 = false;
    if (exp(_7_inputVal.x) == _7_expected.x)
    {
        float2 _39 = exp(_7_inputVal.xy);
        _49 = all(bool2(_39.x == _7_expected.xy.x, _39.y == _7_expected.xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _63 = false;
    if (_49)
    {
        float3 _52 = exp(_7_inputVal.xyz);
        _63 = all(bool3(_52.x == _7_expected.xyz.x, _52.y == _7_expected.xyz.y, _52.z == _7_expected.xyz.z));
    }
    else
    {
        _63 = false;
    }
    bool _74 = false;
    if (_63)
    {
        float4 _66 = exp(_7_inputVal);
        _74 = all(bool4(_66.x == _7_expected.x, _66.y == _7_expected.y, _66.z == _7_expected.z, _66.w == _7_expected.w));
    }
    else
    {
        _74 = false;
    }
    bool _82 = false;
    if (_74)
    {
        _82 = 1.0f == _7_expected.x;
    }
    else
    {
        _82 = false;
    }
    bool _91 = false;
    if (_82)
    {
        _91 = all(bool2(1.0f.xx.x == _7_expected.xy.x, 1.0f.xx.y == _7_expected.xy.y));
    }
    else
    {
        _91 = false;
    }
    bool _100 = false;
    if (_91)
    {
        _100 = all(bool3(1.0f.xxx.x == _7_expected.xyz.x, 1.0f.xxx.y == _7_expected.xyz.y, 1.0f.xxx.z == _7_expected.xyz.z));
    }
    else
    {
        _100 = false;
    }
    bool _108 = false;
    if (_100)
    {
        _108 = all(bool4(1.0f.xxxx.x == _7_expected.x, 1.0f.xxxx.y == _7_expected.y, 1.0f.xxxx.z == _7_expected.z, 1.0f.xxxx.w == _7_expected.w));
    }
    else
    {
        _108 = false;
    }
    float4 _109 = 0.0f.xxxx;
    if (_108)
    {
        _109 = _7_colorGreen;
    }
    else
    {
        _109 = _7_colorRed;
    }
    return _109;
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
