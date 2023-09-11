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
    if (exp2(_7_inputVal.x) == _7_expected.x)
    {
        float2 _39 = exp2(_7_inputVal.xy);
        _49 = all(bool2(_39.x == _7_expected.xy.x, _39.y == _7_expected.xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _63 = false;
    if (_49)
    {
        float3 _52 = exp2(_7_inputVal.xyz);
        _63 = all(bool3(_52.x == _7_expected.xyz.x, _52.y == _7_expected.xyz.y, _52.z == _7_expected.xyz.z));
    }
    else
    {
        _63 = false;
    }
    bool _74 = false;
    if (_63)
    {
        float4 _66 = exp2(_7_inputVal);
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
    bool _92 = false;
    if (_82)
    {
        _92 = all(bool2(float2(1.0f, 2.0f).x == _7_expected.xy.x, float2(1.0f, 2.0f).y == _7_expected.xy.y));
    }
    else
    {
        _92 = false;
    }
    bool _102 = false;
    if (_92)
    {
        _102 = all(bool3(float3(1.0f, 2.0f, 4.0f).x == _7_expected.xyz.x, float3(1.0f, 2.0f, 4.0f).y == _7_expected.xyz.y, float3(1.0f, 2.0f, 4.0f).z == _7_expected.xyz.z));
    }
    else
    {
        _102 = false;
    }
    bool _111 = false;
    if (_102)
    {
        _111 = all(bool4(float4(1.0f, 2.0f, 4.0f, 8.0f).x == _7_expected.x, float4(1.0f, 2.0f, 4.0f, 8.0f).y == _7_expected.y, float4(1.0f, 2.0f, 4.0f, 8.0f).z == _7_expected.z, float4(1.0f, 2.0f, 4.0f, 8.0f).w == _7_expected.w));
    }
    else
    {
        _111 = false;
    }
    float4 _112 = 0.0f.xxxx;
    if (_111)
    {
        _112 = _7_colorGreen;
    }
    else
    {
        _112 = _7_colorRed;
    }
    return _112;
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
