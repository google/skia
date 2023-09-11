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
    if (log(_7_inputVal.x + sqrt(_7_inputVal.x * _7_inputVal.x + 1.0f)) == _7_expected.x)
    {
        float2 _39 = log(_7_inputVal.xy + sqrt(_7_inputVal.xy * _7_inputVal.xy + 1.0f));
        float2 _39 = log(_7_inputVal.xy + sqrt(_7_inputVal.xy * _7_inputVal.xy + 1.0f));
        _49 = all(bool2(_39.x == _7_expected.xy.x, _39.y == _7_expected.xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _63 = false;
    if (_49)
    {
        float3 _52 = log(_7_inputVal.xyz + sqrt(_7_inputVal.xyz * _7_inputVal.xyz + 1.0f));
        float3 _52 = log(_7_inputVal.xyz + sqrt(_7_inputVal.xyz * _7_inputVal.xyz + 1.0f));
        _63 = all(bool3(_52.x == _7_expected.xyz.x, _52.y == _7_expected.xyz.y, _52.z == _7_expected.xyz.z));
    }
    else
    {
        _63 = false;
    }
    bool _74 = false;
    if (_63)
    {
        float4 _66 = log(_7_inputVal + sqrt(_7_inputVal * _7_inputVal + 1.0f));
        float4 _66 = log(_7_inputVal + sqrt(_7_inputVal * _7_inputVal + 1.0f));
        _74 = all(bool4(_66.x == _7_expected.x, _66.y == _7_expected.y, _66.z == _7_expected.z, _66.w == _7_expected.w));
    }
    else
    {
        _74 = false;
    }
    bool _81 = false;
    if (_74)
    {
        _81 = 0.0f == _7_expected.x;
    }
    else
    {
        _81 = false;
    }
    bool _89 = false;
    if (_81)
    {
        _89 = all(bool2(0.0f.xx.x == _7_expected.xy.x, 0.0f.xx.y == _7_expected.xy.y));
    }
    else
    {
        _89 = false;
    }
    bool _99 = false;
    if (_89)
    {
        _99 = all(bool3(float3(0.0f, 0.0f, 1.0f).x == _7_expected.xyz.x, float3(0.0f, 0.0f, 1.0f).y == _7_expected.xyz.y, float3(0.0f, 0.0f, 1.0f).z == _7_expected.xyz.z));
    }
    else
    {
        _99 = false;
    }
    bool _108 = false;
    if (_99)
    {
        _108 = all(bool4(float4(0.0f, 0.0f, 1.0f, -1.0f).x == _7_expected.x, float4(0.0f, 0.0f, 1.0f, -1.0f).y == _7_expected.y, float4(0.0f, 0.0f, 1.0f, -1.0f).z == _7_expected.z, float4(0.0f, 0.0f, 1.0f, -1.0f).w == _7_expected.w));
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
