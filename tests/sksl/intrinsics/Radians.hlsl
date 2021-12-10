cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputVal : packoffset(c0);
    float4 _10_expected : packoffset(c1);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool _51 = false;
    if (radians(_10_inputVal.x) == _10_expected.x)
    {
        float2 _41 = radians(_10_inputVal.xy);
        _51 = all(bool2(_41.x == _10_expected.xy.x, _41.y == _10_expected.xy.y));
    }
    else
    {
        _51 = false;
    }
    bool _65 = false;
    if (_51)
    {
        float3 _54 = radians(_10_inputVal.xyz);
        _65 = all(bool3(_54.x == _10_expected.xyz.x, _54.y == _10_expected.xyz.y, _54.z == _10_expected.xyz.z));
    }
    else
    {
        _65 = false;
    }
    bool _76 = false;
    if (_65)
    {
        float4 _68 = radians(_10_inputVal);
        _76 = all(bool4(_68.x == _10_expected.x, _68.y == _10_expected.y, _68.z == _10_expected.z, _68.w == _10_expected.w));
    }
    else
    {
        _76 = false;
    }
    bool _83 = false;
    if (_76)
    {
        _83 = 0.0f == _10_expected.x;
    }
    else
    {
        _83 = false;
    }
    bool _91 = false;
    if (_83)
    {
        _91 = all(bool2(0.0f.xx.x == _10_expected.xy.x, 0.0f.xx.y == _10_expected.xy.y));
    }
    else
    {
        _91 = false;
    }
    bool _100 = false;
    if (_91)
    {
        _100 = all(bool3(0.0f.xxx.x == _10_expected.xyz.x, 0.0f.xxx.y == _10_expected.xyz.y, 0.0f.xxx.z == _10_expected.xyz.z));
    }
    else
    {
        _100 = false;
    }
    bool _108 = false;
    if (_100)
    {
        _108 = all(bool4(0.0f.xxxx.x == _10_expected.x, 0.0f.xxxx.y == _10_expected.y, 0.0f.xxxx.z == _10_expected.z, 0.0f.xxxx.w == _10_expected.w));
    }
    else
    {
        _108 = false;
    }
    float4 _109 = 0.0f.xxxx;
    if (_108)
    {
        _109 = _10_colorGreen;
    }
    else
    {
        _109 = _10_colorRed;
    }
    return _109;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
